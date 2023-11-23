/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "plugins/accounts/passwd-wrapper.h"
#include <config.h>
#include <fmt/format.h>
#include <glib/gi18n.h>
#include "lib/base/base.h"
#include "plugins/accounts/accounts-util.h"
#include "plugins/accounts/user.h"

namespace Kiran
{
// passwd设置密码错误时运行时间较长，因此这里设置命令超时时间为5秒
#define PASSWD_RUN_TIMEOUT_MSEC 5000

PasswdWrapper::PasswdWrapper(std::weak_ptr<User> user) : user_(user),
                                                         state_(PASSWD_STATE_NONE)
{
}

PasswdWrapper::~PasswdWrapper()
{
    this->free_resources();
}

void PasswdWrapper::exec(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
                         const Glib::ustring &current_password,
                         const Glib::ustring &new_password)
{
    auto user = this->user_.lock();
    if (!user)
    {
        this->end_passwd(false);
        return;
    }

    int32_t caller_uid = 0;
    int standard_input = 0;
    int standard_output = 0;
    int standard_error = 0;
    std::vector<Glib::ustring> argv{"/usr/bin/passwd"};

    this->current_password_ = current_password;
    this->new_password_ = new_password;
    this->passwd_timeout_ = Glib::signal_timeout().connect(sigc::mem_fun(this, &PasswdWrapper::on_passwd_timeout), PASSWD_RUN_TIMEOUT_MSEC);

    AccountsUtil::get_caller_uid(invocation, caller_uid);
    if (caller_uid != (int32_t)user->uid_get())
    {
        argv.push_back(user->user_name_get());
    }

    try
    {
        std::vector<Glib::ustring> envp;
        Glib::spawn_async_with_pipes(Glib::ustring(),
                                     argv,
                                     envp,
                                     Glib::SPAWN_DO_NOT_REAP_CHILD,
                                     std::bind(sigc::mem_fun(this, &PasswdWrapper::on_child_setup), caller_uid),
                                     &this->child_pid_,
                                     &standard_input,
                                     &standard_output,
                                     &standard_error);

        this->in_io_channel_ = Glib::IOChannel::create_from_fd(standard_input);
        this->out_io_channel_ = Glib::IOChannel::create_from_fd(standard_output);
        this->err_io_channel_ = Glib::IOChannel::create_from_fd(standard_error);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_ACCOUNTS("Failed to run passwd: %s.", e.what().c_str());
        this->end_passwd(false);
        return;
    }

    if (!this->init_iochannel(this->in_io_channel_) ||
        !this->init_iochannel(this->out_io_channel_) ||
        !this->init_iochannel(this->err_io_channel_))
    {
        this->end_passwd(false);
        return;
    }

    this->out_io_source_ = this->out_io_channel_->create_watch(Glib::IOCondition::IO_IN | Glib::IOCondition::IO_PRI);
    this->out_io_connection_ = this->out_io_source_->connect(sigc::bind(sigc::mem_fun(this, &PasswdWrapper::on_passwd_output), this->out_io_channel_));
    this->out_io_source_->attach(Glib::MainContext::get_default());

    this->err_io_source_ = this->err_io_channel_->create_watch(Glib::IOCondition::IO_IN | Glib::IOCondition::IO_PRI);
    this->err_io_connection_ = this->err_io_source_->connect(sigc::bind(sigc::mem_fun(this, &PasswdWrapper::on_passwd_output), this->err_io_channel_));
    this->err_io_source_->attach(Glib::MainContext::get_default());

    this->watch_child_connection_ = Glib::signal_child_watch().connect(sigc::mem_fun(this, &PasswdWrapper::on_child_watch), this->child_pid_);

    return;
}

bool PasswdWrapper::init_iochannel(Glib::RefPtr<Glib::IOChannel> io_channel)
{
    try
    {
        std::string null_encoding;
        if (io_channel->set_encoding(null_encoding) != Glib::IO_STATUS_NORMAL)
        {
            KLOG_WARNING_ACCOUNTS("Failed to set encoding for iochannel.");
            return false;
        }

        if (io_channel->set_flags(Glib::IO_FLAG_NONBLOCK) != Glib::IO_STATUS_NORMAL)
        {
            KLOG_WARNING_ACCOUNTS("Failed to set noblock flags for iochannel.");
            return false;
        }
        io_channel->set_buffered(false);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_ACCOUNTS("%s.", e.what().c_str());
        return false;
    }
    return true;
}

void PasswdWrapper::on_child_setup(uint32_t caller_uid)
{
    auto user = this->user_.lock();
    RETURN_IF_FALSE(user);

    // 如果是设置当前用户密码，则需要进行降权
    if (caller_uid == user->uid_get())
    {
        // 必须先设置gid然后再设置uid，否则在设置uid后已经不是特权用户，无法设置gid
        if (setgid(user->gid_get()) != 0 ||
            setuid(user->uid_get()) != 0)
        {
            exit(1);
        }
    }
}

bool PasswdWrapper::on_passwd_output(Glib::IOCondition io_condition, Glib::RefPtr<Glib::IOChannel> io_channel)
{
    Glib::ustring passwd_tips_part;
    Glib::ustring remaining_tips;

    try
    {
        if (io_channel->read(passwd_tips_part, 512) != Glib::IO_STATUS_NORMAL)
        {
            KLOG_WARNING_ACCOUNTS("Failed to read data from IO channel.");
            return true;
        }
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING_ACCOUNTS("IO Channel read error: %s.", e.what().c_str());
        return true;
    }

    this->unhandled_passwd_tips_ += passwd_tips_part;
    KLOG_DEBUG_ACCOUNTS("Read string from passwd command: %s.", passwd_tips_part.c_str());

    // 如果处理已经结束，则只读取缓存数据
    RETURN_VAL_IF_TRUE(this->state_ == PasswdState::PASSWD_STATE_END, true);

    do
    {
        // 记录是否存在换行符号，存在换行符的信息都是报错信息或者其他附加提示信息。
        bool has_line_char = false;
        // 将要被处理的输出，如果有一整行数据，则提取一整行，否则提取剩余部分
        Glib::ustring handled_passwd_tips;
        // 如果一次性读到了多行，则一行一行的处理，因为root设置密码报错后不会终止程序
        auto line_char_pos = this->unhandled_passwd_tips_.find_first_of('\n');
        if (line_char_pos != std::string::npos)
        {
            handled_passwd_tips = this->unhandled_passwd_tips_.substr(0, line_char_pos + 1);
            this->unhandled_passwd_tips_ = this->unhandled_passwd_tips_.substr(line_char_pos + 1);
            has_line_char = true;
        }
        else
        {
            handled_passwd_tips = this->unhandled_passwd_tips_;
            this->unhandled_passwd_tips_.clear();
        }

        auto retval = this->process_passwd_output_line(handled_passwd_tips);

        // 处理出错且后面不再有数据则退出
        if (!this->additional_error_message_.empty() && this->unhandled_passwd_tips_.empty())
        {
            this->end_passwd(false);
            break;
        }

        // 如果没有处理整行数据且处理失败，说明数据没有读取完整，因此将数据重新放回取等待下次处理
        if (!retval && !has_line_char)
        {
            this->unhandled_passwd_tips_ = handled_passwd_tips;
            break;
        }

        // 数据已经处理完毕，跳出循环
        if (this->unhandled_passwd_tips_.empty())
        {
            break;
        }

    } while (true);

    return true;
}

bool PasswdWrapper::process_passwd_output_line(const std::string &line)
{
    bool retval = false;
    auto lowercase_passwd_tips = StrUtils::tolower(line);

    KLOG_DEBUG_ACCOUNTS("Process string content is: %s.", line.c_str());

    switch (this->state_)
    {
    case PASSWD_STATE_NONE:
        // 设置新密码
        if (StrUtils::endswith(lowercase_passwd_tips, "new password: "))
        {
            this->state_ = PASSWD_STATE_NEW;
            this->in_io_channel_->write(this->new_password_ + "\n");
            retval = true;
        }
        // 填写当前密码，root用户下不会出现这一步
        else if (StrUtils::endswith(lowercase_passwd_tips, "current password: "))
        {
            this->state_ = PASSWD_STATE_AUTH;
            this->in_io_channel_->write(this->current_password_ + "\n");
            retval = true;
        }
        // 第一次不会出现错误
        break;
    case PASSWD_STATE_AUTH:
        if (StrUtils::contains_oneof_substrs(lowercase_passwd_tips, std::vector<std::string>{"password: ", "failure", "wrong", "error"}))
        {
            // 认证成功
            if (StrUtils::contains_allof_substrs(lowercase_passwd_tips, std::vector<std::string>{"password: ", "new"}))
            {
                this->state_ = PASSWD_STATE_NEW;
                this->in_io_channel_->write(this->new_password_ + "\n");
            }
            // 认证失败，再次认证
            else if (StrUtils::endswith(lowercase_passwd_tips, "current password: "))
            {
                this->in_io_channel_->write(this->current_password_ + "\n");
            }
            else
            {
                // this->error_desc_ = _("The current password was incorrect.");
                this->additional_error_message_ = this->translation_passwd_tips(line);
            }
            retval = true;
        }
        break;

    case PASSWD_STATE_NEW:
        if (StrUtils::endswith(lowercase_passwd_tips, "retype new password: "))
        {
            this->state_ = PASSWD_STATE_RETYPE;
            this->in_io_channel_->write(this->new_password_ + "\n");
            retval = true;
        }
        /* 如果是整行信息，说明是错误或者告警信息。因为这里没法区分是错误或者告警信息，所以只能继续往下处理到数据结束，
           如果最后一条是提示信息，说明当前属于告警信息，否则是错误信息。*/
        else if (line.find_first_of('\n') != std::string::npos)
        {
            this->state_ = PASSWD_STATE_ERROR;
            this->additional_error_message_ = this->translation_passwd_tips(line);
            retval = true;
        }
        break;
    case PASSWD_STATE_RETYPE:
        if (StrUtils::contains_oneof_substrs(lowercase_passwd_tips, std::vector<std::string>{
                                                                        "successfully",
                                                                        "failure",
                                                                    }))
        {
            if (lowercase_passwd_tips.find("successfully") != std::string::npos)
            {
                // 密码设置成功
                this->end_passwd(true);
            }
            else
            {
                this->additional_error_message_ = this->translation_passwd_tips(line);
                this->state_ = PASSWD_STATE_ERROR;
            }
            retval = true;
        }
        break;
    case PASSWD_STATE_ERROR:
    {
        // 这里说明上一条信息是告警消息而非错误消息，因此清空错误消息并继续往下走
        if (StrUtils::endswith(lowercase_passwd_tips, "retype new password: "))
        {
            this->state_ = PASSWD_STATE_RETYPE;
            this->in_io_channel_->write(this->new_password_ + "\n");
            this->additional_error_message_.clear();
            retval = true;
        }
        break;
    }
    default:
        retval = true;
        break;
    }

    return retval;
}

void PasswdWrapper::on_child_watch(GPid pid, int child_status)
{
    KLOG_DEBUG_ACCOUNTS("Process passwd(%d) exit, exit status is %d.", (int32_t)pid, child_status);

    g_autoptr(GError) g_error = NULL;
    auto result = g_spawn_check_exit_status(child_status, &g_error);
    if (!result)
    {
        KLOG_WARNING_ACCOUNTS("%s.", g_error->message);
        if (this->error_message_.empty())
        {
            this->error_message_ = CC_ERROR2STR(CCErrorCode::ERROR_FAILED);
        }
    }

    this->exec_finished_.emit(this->error_message_);
    this->free_resources();
}

std::string PasswdWrapper::translation_passwd_tips(const std::string &passwd_tips)
{
    auto trim_passwd_tips = StrUtils::trim(passwd_tips);

    if (StrUtils::startswith(trim_passwd_tips, "BAD PASSWORD: "))
    {
        trim_passwd_tips = trim_passwd_tips.substr(strlen("BAD PASSWORD: "));
    }

    if (StrUtils::startswith(trim_passwd_tips, "passwd: "))
    {
        trim_passwd_tips = trim_passwd_tips.substr(strlen("passwd: "));
    }

    KLOG_DEBUG_ACCOUNTS("Trim passwd: %s.", trim_passwd_tips.c_str());

    bool translation_success = true;
    auto trim_passwd_tips_vec = StrUtils::split_with_char(trim_passwd_tips, '-');
    for (uint32_t i = 0; i < trim_passwd_tips_vec.size(); ++i)
    {
        auto trim2_passwd_tips = StrUtils::trim(trim_passwd_tips_vec[i]);
        trim_passwd_tips_vec[i] = this->translation_with_gettext(trim2_passwd_tips);
        if (trim_passwd_tips_vec[i].empty())
        {
            translation_success = false;
            break;
        }
    }

    if (translation_success)
    {
        return StrUtils::join(trim_passwd_tips_vec, " - ");
    }

#define MATCH_WITH_ONE_NUMBER(pattern, translation)                                     \
    {                                                                                   \
        Glib::MatchInfo match_info;                                                     \
        auto regex = Glib::Regex::create(pattern);                                      \
        if (regex->match(passwd_tips, match_info) && match_info.get_match_count() == 2) \
        {                                                                               \
            auto number = std::stoi(match_info.fetch(1).raw());                         \
            return fmt::format(translation, number);                                    \
        }                                                                               \
    }

    MATCH_WITH_ONE_NUMBER("less than ([0-9]+) character classes", _("The password contains less than {0} character classes."));
    MATCH_WITH_ONE_NUMBER("less than ([0-9]+) digits", _("The password contains less than {0} digits."));
    MATCH_WITH_ONE_NUMBER("less than ([0-9]+) lowercase letters", _("The password contains less than {0} lowercase letters."));
    MATCH_WITH_ONE_NUMBER("less than ([0-9]+) non-alphanumeric characters", _("The password contains less than {0} non-alphanumeric characters."));
    MATCH_WITH_ONE_NUMBER("less than ([0-9]+) uppercase letters", _("The password contains less than {0} uppercase letters."));
    MATCH_WITH_ONE_NUMBER("monotonic sequence longer than ([0-9]+) characters", _("The password contains monotonic sequence longer than {0} characters."));
    MATCH_WITH_ONE_NUMBER("more than ([0-9]+) characters of the same class consecutively", _("The password contains more than {0} characters of the same class consecutively."));
    MATCH_WITH_ONE_NUMBER("more than ([0-9]+) same characters consecutively", _("The password contains more than {0} same characters consecutively."));
    MATCH_WITH_ONE_NUMBER("shorter than ([0-9]+) characters", _("The password is shorter than {0} characters."));

    return trim_passwd_tips;
}

std::string PasswdWrapper::translation_with_gettext(const std::string &message_id)
{
    KLOG_DEBUG_ACCOUNTS("Translation message '%s' with gettext.", message_id.c_str());

#define TRANS_WITH_DOMAIN(domainname, text)                                     \
    do                                                                          \
    {                                                                           \
        BREAK_IF_TRUE(bindtextdomain(domainname, "/usr/share/locale") == NULL); \
        BREAK_IF_TRUE(bind_textdomain_codeset(domainname, "UTF-8") == NULL);    \
        auto translated_text = dgettext(domainname, text.c_str());              \
        BREAK_IF_TRUE(translated_text == text.c_str())                          \
        return translated_text;                                                 \
    } while (0)

    TRANS_WITH_DOMAIN("libpwquality", message_id);
    TRANS_WITH_DOMAIN("Linux-PAM", message_id);
    TRANS_WITH_DOMAIN("cracklib", message_id);

    return std::string();
}

void PasswdWrapper::end_passwd(bool is_success)
{
    KLOG_DEBUG_ACCOUNTS("The command of passwd execution completed.");

    if (!is_success)
    {
        if (this->additional_error_message_.empty())
        {
            this->error_message_ = CC_ERROR2STR(CCErrorCode::ERROR_FAILED);
        }
        else
        {
            this->error_message_ = fmt::format(CC_ERROR2STR(CCErrorCode::ERROR_ACCOUNTS_USER_MODIFY_PASSWORD_FAILED), this->additional_error_message_);
        }
    }

    if (this->child_pid_ == 0)
    {
        this->exec_finished_.emit(this->error_message_);
        this->free_resources();
    }
    /* 如果进程还未退出，则等待进程退出后再发送错误消息，因为进程退出可能会有1-2秒的时间，
    如果先发送了错误消息，用户立即设置下一次密码时会收到”其他用户正在设置密码“的错误消息。*/
    else
    {
        this->state_ = PasswdState::PASSWD_STATE_END;
    }
}

bool PasswdWrapper::on_passwd_timeout()
{
    KLOG_WARNING_ACCOUNTS("Passwd run timeout.");

    /* 在收到passwd的错误消息后，大概会等1-2秒进程才会退出，如果此时超时定时器被触发，
    应该要判断一下是否是已经处理结束，如果是的话，使用之前的错误消息*/
    if (this->state_ != PasswdState::PASSWD_STATE_END)
    {
        this->additional_error_message_ = _("Password modification timeout.");
        this->end_passwd(false);
    }
    this->stop_passwd();
    return false;
}

void PasswdWrapper::stop_passwd()
{
    if (this->child_pid_ != 0)
    {
        kill(this->child_pid_, 9);
        this->child_pid_ = 0;
    }
}

void PasswdWrapper::free_resources()
{
    this->state_ = PASSWD_STATE_NONE;
    this->unhandled_passwd_tips_.clear();

    this->current_password_.clear();
    this->new_password_.clear();
    this->additional_error_message_.clear();
    this->error_message_.clear();

    this->watch_child_connection_.disconnect();

    /* passwd运行完毕后需要停止句柄的监听，否则在调用spawn_close_pid时，如果还存在输出进入到到管道，
       会调用on_passwd_output回调函数，而此时进程已经进入退出阶段，此时对管道进行写入会出现未知的异常情况。*/
    if (this->out_io_source_)
    {
        this->out_io_source_->destroy();
    }

    if (this->err_io_source_)
    {
        this->err_io_source_->destroy();
    }

    if (this->child_pid_)
    {
        Glib::spawn_close_pid(this->child_pid_);
        this->child_pid_ = 0;
    }

    this->out_io_connection_.disconnect();
    this->err_io_connection_.disconnect();

    this->in_io_channel_.reset();
    this->out_io_channel_.reset();
    this->err_io_channel_.reset();
    this->out_io_source_.reset();
    this->err_io_source_.reset();

    this->passwd_timeout_.disconnect();
}
}  // namespace Kiran
