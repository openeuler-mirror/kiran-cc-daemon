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
 * Author:     meizhigang <meizhigang@kylinsec.com.cn>
 */
#include <gtkmm.h>
namespace Kiran
{
class ModifierLockWindow : public Gtk::Window
{
public:
    ModifierLockWindow();
    ~ModifierLockWindow();

    void show_capslock_on();

    void show_capslock_off();

    void show_numlock_on();

    void show_numlock_off();

private:
    void init();

    void on_real_show();

    void on_real_hide();

    bool on_real_draw(const Cairo::RefPtr<Cairo::Context>& rc);

    bool on_hide_timeout();

    void add_hide_timeout();

    void remove_hide_timeout();

private:
    std::string image_file_;

    sigc::connection hide_timeout_id_;
};
}  // namespace Kiran