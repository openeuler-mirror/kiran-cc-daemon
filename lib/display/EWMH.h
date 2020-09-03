/*
 * @Author       : tangjie02
 * @Date         : 2020-09-02 15:43:07
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-09-03 09:00:44
 * @Description  : 
 * @FilePath     : /kiran-cc-daemon/lib/display/EWMH.h
 */
#pragma once
#include <gdkmm.h>
//
#include <X11/X.h>

namespace Kiran
{
// Extended Window Manager Hints
class EWMH
{
public:
    EWMH();
    virtual ~EWMH();

    static EWMH* get_instance() { return instance_; };

    static void global_init();

    static void global_deinit();

    // 获得窗口管理器keybingding名字列表
    std::vector<std::string> get_wm_keybindings();

    // 获取窗口管理器的窗口属性
    std::string get_wm_property(Atom atom);

    sigc::signal<void> signal_wm_window_change() { return this->wm_window_changed_; }

private:
    void init();

    void update_wm_window();

    static GdkFilterReturn window_event(GdkXEvent* gdk_event, GdkEvent* event, gpointer data);

private:
    static EWMH* instance_;

    Window wm_window_;

    sigc::signal<void> wm_window_changed_;
};
}  // namespace Kiran