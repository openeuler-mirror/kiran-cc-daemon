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

#pragma once

#include <giomm.h>

namespace Kiran
{
enum PasswdState
{
    PASSWD_STATE_NONE,
    // 请求当前密码
    PASSWD_STATE_AUTH,
    // 请求新密码
    PASSWD_STATE_NEW,
    // 请求确认密码
    PASSWD_STATE_RETYPE,
    // 设置密码出现错误
    PASSWD_STATE_ERROR,
    // 运行结束
    PASSWD_STATE_END
};

class User;

class PasswdWrapper : public sigc::trackable
{
public:
    PasswdWrapper(std::weak_ptr<User> user);
    virtual ~PasswdWrapper();

    // 设置密码当前进度/状态
    PasswdState get_state() { return this->state_; };

    // 执行passwd命令
    void exec(Glib::RefPtr<Gio::DBus::MethodInvocation> invocation,
              const Glib::ustring &current_password,
              const Glib::ustring &new_password);

    // 命令执行完毕
    sigc::signal<void, const std::string &> &signal_exec_finished() { return this->exec_finished_; };

private:
    bool init_iochannel(Glib::RefPtr<Glib::IOChannel> io_channel);

    void on_child_setup(uint32_t caller_uid);
    bool on_passwd_output(Glib::IOCondition io_condition, Glib::RefPtr<Glib::IOChannel> io_channel);
    bool process_passwd_output_line(const std::string &line);
    void on_child_watch(GPid pid, int child_status);
    std::string translation_passwd_tips(const std::string &passwd_tips);
    std::string translation_with_gettext(const std::string &message_id);
    // 正常退出
    void end_passwd(bool is_success);
    bool on_passwd_timeout();
    // 中途终止退出
    void stop_passwd();
    void free_resources();

private:
    std::weak_ptr<User> user_;
    PasswdState state_;
    // 剩余未处理的passwd命令输出数据
    Glib::ustring unhandled_passwd_tips_;
    Glib::Pid child_pid_;
    Glib::RefPtr<Glib::IOChannel> in_io_channel_;
    Glib::RefPtr<Glib::IOChannel> out_io_channel_;
    Glib::RefPtr<Glib::IOChannel> err_io_channel_;
    Glib::RefPtr<Glib::IOSource> out_io_source_;
    Glib::RefPtr<Glib::IOSource> err_io_source_;
    sigc::connection out_io_connection_;
    sigc::connection err_io_connection_;

    Glib::ustring current_password_;
    Glib::ustring new_password_;
    Glib::ustring additional_error_message_;
    Glib::ustring error_message_;

    sigc::connection watch_child_connection_;
    sigc::connection passwd_timeout_;

    sigc::signal<void, const std::string &> exec_finished_;
};
}  // namespace Kiran