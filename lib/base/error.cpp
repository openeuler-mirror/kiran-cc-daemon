/**
 * @file          /kiran-cc-daemon/lib/base/error.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/base/error.h"

#include <fmt/format.h>
#include <glib/gi18n.h>

namespace Kiran
{
CCError::CCError()
{
}

std::string CCError::get_error_desc(CCErrorCode error_code)
{
    std::string error_desc;
    switch (error_code)
    {
    case CCErrorCode::ERROR_PLUGIN_NOT_EXIST_1:
    case CCErrorCode::ERROR_PLUGIN_NOT_EXIST_2:
        error_desc = _("The plugin doesn't exist.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_1:
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_2:
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_3:
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_4:
        error_desc = _("No user found.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_ALREADY_EXIST:
        error_desc = _("The user already exists.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_UNKNOWN_ACCOUNT_TYPE:
        error_desc = _("Unknown account type.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_PW_UPDATE:
        error_desc = _("Can't update password file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USAGE:
        error_desc = _("Invalid command syntax.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_ARG:
        error_desc = _("Invalid argument to option.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UID_IN_USE:
        error_desc = _("UID already in use.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_PWFILE:
        error_desc = _("Passwd file contains errors.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOTFOUND:
        error_desc = _("Specified user/group doesn't exist.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USER_BUSY:
        error_desc = _("User to modify is logged in.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NAME_IN_USE:
        error_desc = _("Username already in use.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_GRP_UPDATE:
        error_desc = _("Can't update group file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOSPACE:
        error_desc = _("Insufficient space to move home dir.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_HOMEDIR:
        error_desc = _("Can't create/remove/move home directory.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_1:
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_2:
        error_desc = _("Can't update SELinux user mapping.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_UID_UPDATE:
        error_desc = _("Can't update the subordinate uid file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_GID_UPDATE:
        error_desc = _("Can't update the subordinate gid file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_DELETE_ROOT_USER:
        error_desc = _("Refuse to delete root user.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_1:
    case CCErrorCode::ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_2:
    case CCErrorCode::ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_3:
    case CCErrorCode::ERROR_ACCOUNTS_AUTHENTICATION_UNSUPPORTED_4:
        error_desc = _("The authentication mode isn't supported.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_IS_LOCKED:
        error_desc = _("User is locked.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_EXPIRATION_POLICY_NOTFOUND:
        error_desc = _("The expiration policy isn't found.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_AUTHMODE_NAME_ALREADY_EXIST:
        error_desc = _("The name already exists.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEVICE_VOLUME_RANGE_INVLAID:
    case CCErrorCode::ERROR_AUDIO_STREAM_VOLUME_RANGE_INVLAID:
        error_desc = _("The range of volume is between 0 and 1.0.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEVICE_BALANCE_RANGE_INVLAID:
    case CCErrorCode::ERROR_AUDIO_DEVICE_FADE_RANGE_INVLAID:
        error_desc = _("The range of balance is between -1 and 1.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEFAULT_SINK_NOT_FOUND:
    case CCErrorCode::ERROR_AUDIO_SINK_NOT_FOUND:
        error_desc = _("The sink device isn't found.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEFAULT_SOURCE_NOT_FOUND:
    case CCErrorCode::ERROR_AUDIO_SOURCE_NOT_FOUND:
        error_desc = _("The source device isn't found.");
        break;
    case CCErrorCode::ERROR_AUDIO_SINK_INPUT_NOT_FOUND:
    case CCErrorCode::ERROR_AUDIO_SOURCE_OUTPUT_NOT_FOUND:
        error_desc = _("The sink stream isn't found.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_NTP_IS_ACTIVE:
        error_desc = _("NTP unit is active.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_TIMEZONE_INVALIDE:
        error_desc = _("Invalid timezone.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_NO_NTP_UNIT:
        error_desc = _("No NTP unit available.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_1:
    case CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_2:
        error_desc = _("Unknown date format type.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_SET_DATE_FORMAT_FAILED:
        error_desc = _("Failed to set date format.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_SET_HOUR_FORMAT_FAILED:
        error_desc = _("Failed to set hour format.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_SET_SECONDS_SHOWING_FAILED:
        error_desc = _("Failed to set seconds showing.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_START_NTP_FAILED:
        error_desc = _("Failed to start NTP unit.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_STOP_NTP_FAILED:
        error_desc = _("Failed to stop NTP unit.");
        break;
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_1:
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_2:
        error_desc = _("Unknown display style.");
        break;
    case CCErrorCode::ERROR_DISPLAY_COMMON_MODE_NOTFOUND:
        error_desc = _("The mode of monitors which contain resolution and refresh rate is no intersection.");
        break;
    case CCErrorCode::ERROR_DISPLAY_SET_AUTO_MODE_FAILED:
        error_desc = _("Auto mode is set failed.");
        break;
    case CCErrorCode::ERROR_DISPLAY_SET_WINDOW_SCALING_FACTOR_1:
    case CCErrorCode::ERROR_DISPLAY_SET_WINDOW_SCALING_FACTOR_2:
        error_desc = _("Failed to set the window scaling factor.");
        break;
    case CCErrorCode::ERROR_DISPLAY_CONFIG_IS_EMPTY:
        error_desc = _("The custom configuration file isn't found.");
        break;
    case CCErrorCode::ERROR_DISPLAY_CONFIG_ITEM_NOTFOUND:
        error_desc = _("Not found matched item in custom configuration file.");
        break;
    case CCErrorCode::ERROR_DISPLAY_PRIMARY_MONITOR_IS_EMPTY:
        error_desc = _("The primary monitor must not be empty.");
        break;
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_PRIMARY_MONITOR_BY_NAME:
        error_desc = _("Not found the primary monitor.");
        break;
    case CCErrorCode::ERROR_DISPLAY_ONLY_ONE_ENABLED_MONITOR:
        error_desc = _("Cannot disable the monitor, because the number of the enabled monitor is less than 1.");
        break;
    case CCErrorCode::ERROR_DISPLAY_EXIST_NULL_MODE_IN_LIST:
        error_desc = _("Exist null mode in mode list.");
        break;
    case CCErrorCode::ERROR_DISPLAY_EXIST_NULL_MODE_IN_PREFER_LIST:
        error_desc = _("Exist null mode in preferred mode list.");
        break;
    case CCErrorCode::ERROR_DISPLAY_MODE_NOT_EXIST:
        error_desc = _("The current mode is not exist.");
        break;
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_MATCH_MODE_1:
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_MATCH_MODE_2:
        error_desc = _("Not found match mode.");
        break;
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_MODE_BY_ID:
        error_desc = _("The mode is not exist.");
        break;
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_ROTATION_TYPE:
        error_desc = _("Unknown rotation type.");
        break;
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_REFLECT_TYPE:
        error_desc = _("Unknown reflect type.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_THEME_NOT_EXIST:
        error_desc = _("Theme not exist.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_UNSUPPORTED:
        error_desc = _("Unsupported theme type.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_INVALID:
        error_desc = _("Invalid theme type.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_1:
    case CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_2:
        error_desc = _("Invalid font type.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_UNSUPPORTED:
        error_desc = _("Unsupported font type.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_EXIST_REQUEST_INCOMPLETE:
        error_desc = _("An incomplete request already exists.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_CANCELED:
        error_desc = _("The request is canceled.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_1:
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_2:
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_3:
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_4:
        error_desc = _("The request is rejected.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_NOTFOUND_ADAPTOR:
        error_desc = _("Not found adapter.");
        break;
    case CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_1:
    case CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_2:
    case CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_3:
    case CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_4:
    case CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_5:
    case CCErrorCode::ERROR_GREETER_SYNC_TO_FILE_FAILED_6:
        error_desc = _("Sync to file failed.");
        break;
    // case CCErrorCode::ERROR_GREETER_NOTFOUND_USER:
    //     error_desc = _("Failed to find user name.");
    //     break;
    case CCErrorCode::ERROR_GREETER_SCALE_MODE_INVALIDE:
        error_desc = _("Invalid scale mode.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_EXCEED_LIMIT:
        error_desc = _("The number of the layout can't exceeds {0}.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_INVALID:
        error_desc = _("The layout is invalid.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_ALREADY_EXIST:
        error_desc = _("The layout already exist.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_SET_FAILED:
        error_desc = _("Failed to set the layout.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_NOT_EXIST:
        error_desc = _("The layout is no exist.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_UPDATE_FAILED:
        error_desc = _("Failed to update the layout.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_ALREADY_EXIST:
        error_desc = _("The layout option already exist.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_SET_FAILED:
        error_desc = _("Failed to set the layout option.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_NOT_EXIST:
        error_desc = _("The layout option is no exist.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_UPDATE_FAILED:
        error_desc = _("Failed to update the layout option.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_CLEAR_FAILED:
        error_desc = _("Failed to clear the layout option.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_INVALID:
        error_desc = _("The custom shortcut is invalid.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_1:
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_2:
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST_3:
        error_desc = _("The custom shortcut isn't exist.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_ALREADY_EXIST:
        error_desc = _("The custom shortcut already exist.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST_1:
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST_2:
        error_desc = _("The key combination already exist.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID_1:
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID_2:
    case CCErrorCode::ERROR_KEYBINDING_SYSTEM_KEYCOMB_INVALID_1:
        error_desc = _("The key combination is invalid.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST_1:
    case CCErrorCode::ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST_2:
        error_desc = _("The system shortcut isn't exist.");
        break;
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_1:
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_2:
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_3:
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_4:
        error_desc = _("Unsupported power supply mode.");
        break;
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_1:
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_2:
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_3:
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_4:
        error_desc = _("Unsupported power device.");
        break;
    case CCErrorCode::ERROR_POWER_DIMMED_SCALE_RANGE_ERROR:
        error_desc = _("The value must be between 0 and 100.");
        break;
    case CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_1:
    case CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_2:
        error_desc = _("Unknown power action.");
        break;
    case CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_1:
    case CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_2:
        error_desc = _("Unsupported power event.");
        break;
    case CCErrorCode::ERROR_POWER_SET_ACTION_FAILED:
        error_desc = _("Failed to set the action.");
        break;
    case CCErrorCode::ERROR_POWER_SET_BRIGHTNESS_FAILED:
        error_desc = _("Failed to set brightness.");
        break;
    case CCErrorCode::ERROR_SYSTEMINFO_TYPE_INVALID:
        error_desc = _("The systeminfo type is invalid.");
        break;
    case CCErrorCode::ERROR_SYSTEMINFO_SET_HOSTNAME_FAILED:
        error_desc = _("Failed to set host name.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_NOTFOUND_PROPERTY:
        error_desc = _("Not found the property.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_TYPE_MISMATCH:
        error_desc = _("The type is mismatch.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_INVALID:
        error_desc = _("The property is invalid.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_ONLYREAD:
        error_desc = _("The property must not be modified manually.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_UNSUPPORTED:
        error_desc = _("The property is unsupported.");
        break;

    case CCErrorCode::ERROR_ACCOUNTS_SPAWN_SYNC_FAILED:
    case CCErrorCode::ERROR_ACCOUNTS_SPAWN_EXIT_STATUS:
    case CCErrorCode::ERROR_ACCOUNTS_SAVE_AUTOLOGIN_FILE:
    case CCErrorCode::ERROR_ACCOUNTS_UNKNOWN_CALLER_UID_1:
    case CCErrorCode::ERROR_ACCOUNTS_UNKNOWN_CALLER_UID_2:
    case CCErrorCode::ERROR_ACCOUNTS_QUERY_INFO_FAILED:
    case CCErrorCode::ERROR_ACCOUNTS_FILE_TYPE_NQ_REGULAR:
    case CCErrorCode::ERROR_ACCOUNTS_REPLACE_OUTPUT_STREAM:
    case CCErrorCode::ERROR_ACCOUNTS_SPAWN_READ_FILE_FAILED:
    case CCErrorCode::ERROR_ACCOUNTS_COPY_FILE_FAILED:
    case CCErrorCode::ERROR_ACCOUNTS_GROUP_NOT_FOUND:
    case CCErrorCode::ERROR_ACCOUNTS_AUTH_SAVE_DATA_FAILED:
    case CCErrorCode::ERROR_ACCOUNTS_AUTH_DEL_DATA_FAILED:
    case CCErrorCode::ERROR_DISPLAY_EXEC_XRANDR_FAILED:
    case CCErrorCode::ERROR_DISPLAY_SAVE_CREATE_FILE_FAILED:
    case CCErrorCode::ERROR_DISPLAY_WRITE_CONF_FILE_FAILED:
    case CCErrorCode::ERROR_APPEARANCE_SET_BACKGROUND_FAILED:
    case CCErrorCode::ERROR_KEYBINDING_GEN_UID_FAILED:
    case CCErrorCode::ERROR_KEYBINDING_GRAB_KEY_FAILED:
    case CCErrorCode::ERROR_SYSTEMINFO_JSON_ASSIGN_FAILED:
    case CCErrorCode::ERROR_PLUGIN_NOTFOUND_NEW_PLUGIN_FUNC:
    case CCErrorCode::ERROR_PLUGIN_NOTFOUND_DEL_PLUGIN_FUNC:
    case CCErrorCode::ERROR_PLUGIN_OPEN_MODULE_FAILED:
    case CCErrorCode::ERROR_AUDIO_SET_SINK_ACTIVE_PORT_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_VOLUME_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_BALANCE_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_FADE_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_MUTE_FAILED:
    case CCErrorCode::ERROR_AUDIO_STREAM_SET_VOLUME_FAILED:
    case CCErrorCode::ERROR_AUDIO_STREAM_SET_MUTE_FAILED:
        error_desc = _("Internel error.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UNKNOWN:
        error_desc = _("Unknown error.");
        break;
    default:
        error_desc = _("Unknown error.");
        break;
    }

    error_desc += fmt::format(_(" (error code: 0x{:x})"), int32_t(error_code));
    return error_desc;
}
}  // namespace Kiran
