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

#include "error.h"
#include <fmt/format.h>

namespace Kiran
{
CCError::CCError()
{
}

// sonarqube block off
// 为switch case 简单条件匹配，只是case条件分支过多，暂时关闭扫描。
QString CCError::getErrorDesc(CCErrorCode errorCode, bool attachErrorCode)
{
    QString errorDesc;
    switch (errorCode)
    {
    case CCErrorCode::ERROR_ARGUMENT_INVALID:
        errorDesc = tr("The argument is invalid.");
        break;
    case CCErrorCode::ERROR_CALL_FUNCTION_FAILED:
        errorDesc = tr("Operation failed.");
        break;
    case CCErrorCode::ERROR_PLUGIN_NOT_EXIST:
        errorDesc = tr("The plugin doesn't exist.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_1:
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_2:
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_3:
    case CCErrorCode::ERROR_ACCOUNTS_USER_NOT_FOUND_4:
        errorDesc = tr("No user found.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_ALREADY_LOGIN:
        errorDesc = tr("The user is already logined in, Please log off the user before deleting it.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_ALREADY_EXIST:
        errorDesc = tr("The user already exists.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_UNKNOWN_ACCOUNT_TYPE:
        errorDesc = tr("Unknown account type.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_PW_UPDATE:
        errorDesc = tr("Can't update password file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USAGE:
        errorDesc = tr("Invalid command syntax.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_ARG:
        errorDesc = tr("Invalid argument to option.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UID_IN_USE:
        errorDesc = tr("UID already in use.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_BAD_PWFILE:
        errorDesc = tr("Passwd file contains errors.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOTFOUND:
        errorDesc = tr("Specified user/group doesn't exist.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_USER_BUSY:
        errorDesc = tr("User to modify is logged in.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NAME_IN_USE:
        errorDesc = tr("Username already in use.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_GRP_UPDATE:
        errorDesc = tr("Can't update group file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_NOSPACE:
        errorDesc = tr("Insufficient space to move home dir.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_HOMEDIR:
        errorDesc = tr("Can't create/remove/move home directory.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_1:
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SE_UPDATE_2:
        errorDesc = tr("Can't update SELinux user mapping.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_UID_UPDATE:
        errorDesc = tr("Can't update the subordinate uid file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_SUB_GID_UPDATE:
        errorDesc = tr("Can't update the subordinate gid file.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_DELETE_ROOT_USER:
        errorDesc = tr("Refuse to delete root user.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_DELETE_THREE_AUTH_USER:
        errorDesc = tr("Refuse to delete three authority user.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_IS_LOCKED:
        errorDesc = tr("User is locked.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_MODIFYING_PASSWORD:
        errorDesc = tr("A user is modifying the password.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_MODIFY_PASSWORD_FAILED:
        errorDesc = tr("%1");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USRE_CURRENT_PASSWORD_DISMATCH:
        errorDesc = tr("The current password is dismatch.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEVICE_VOLUME_RANGE_INVLAID:
    case CCErrorCode::ERROR_AUDIO_STREAM_VOLUME_RANGE_INVLAID:
        errorDesc = tr("The range of volume is between 0 and 1.0.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEVICE_BALANCE_RANGE_INVLAID:
    case CCErrorCode::ERROR_AUDIO_DEVICE_FADE_RANGE_INVLAID:
        errorDesc = tr("The range of balance is between -1 and 1.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEFAULT_SINK_NOT_FOUND:
    case CCErrorCode::ERROR_AUDIO_SINK_NOT_FOUND:
        errorDesc = tr("The sink device isn't found.");
        break;
    case CCErrorCode::ERROR_AUDIO_DEFAULT_SOURCE_NOT_FOUND:
    case CCErrorCode::ERROR_AUDIO_SOURCE_NOT_FOUND:
        errorDesc = tr("The source device isn't found.");
        break;
    case CCErrorCode::ERROR_AUDIO_SINK_INPUT_NOT_FOUND:
    case CCErrorCode::ERROR_AUDIO_SOURCE_OUTPUT_NOT_FOUND:
        errorDesc = tr("The sink stream isn't found.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_NTP_IS_ACTIVE:
        errorDesc = tr("NTP unit is active.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_TIMEZONE_INVALIDE:
        errorDesc = tr("Invalid timezone.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_NO_NTP_UNIT:
        errorDesc = tr("No NTP unit available.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_1:
    case CCErrorCode::ERROR_TIMEDATE_UNKNOWN_DATE_FORMAT_TYPE_2:
        errorDesc = tr("Unknown date format type.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_START_NTP_FAILED:
        errorDesc = tr("Failed to start NTP unit.");
        break;
    case CCErrorCode::ERROR_TIMEDATE_STOP_NTP_FAILED:
        errorDesc = tr("Failed to stop NTP unit.");
        break;
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_1:
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_DISPLAY_STYLE_2:
        errorDesc = tr("Unknown display style.");
        break;
    case CCErrorCode::ERROR_DISPLAY_COMMON_MODE_NOTFOUND:
        errorDesc = tr("The mode of monitors which contain resolution and refresh rate is no intersection.");
        break;
    case CCErrorCode::ERROR_DISPLAY_CONFIG_IS_EMPTY:
        errorDesc = tr("The custom configuration file isn't found.");
        break;
    case CCErrorCode::ERROR_DISPLAY_CONFIG_ITEM_NOTFOUND:
        errorDesc = tr("Not found matched item in custom configuration file.");
        break;
    case CCErrorCode::ERROR_DISPLAY_PRIMARY_MONITOR_IS_EMPTY:
        errorDesc = tr("The primary monitor must not be empty.");
        break;
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_PRIMARY_MONITOR_BY_NAME:
        errorDesc = tr("Not found the primary monitor.");
        break;
    case CCErrorCode::ERROR_DISPLAY_ONLY_ONE_ENABLED_MONITOR:
        errorDesc = tr("Cannot disable the monitor, because the number of the enabled monitor is less than 1.");
        break;
    case CCErrorCode::ERROR_DISPLAY_EXIST_NULL_MODE_IN_LIST:
        errorDesc = tr("Exist null mode in mode list.");
        break;
    case CCErrorCode::ERROR_DISPLAY_EXIST_NULL_MODE_IN_PREFER_LIST:
        errorDesc = tr("Exist null mode in preferred mode list.");
        break;
    case CCErrorCode::ERROR_DISPLAY_MODE_NOT_EXIST:
        errorDesc = tr("The current mode is not exist.");
        break;
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_MATCH_MODE_1:
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_MATCH_MODE_2:
        errorDesc = tr("Not found match mode.");
        break;
    case CCErrorCode::ERROR_DISPLAY_NOTFOUND_MODE_BY_ID:
        errorDesc = tr("The mode is not exist.");
        break;
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_ROTATION_TYPE:
        errorDesc = tr("Unknown rotation type.");
        break;
    case CCErrorCode::ERROR_DISPLAY_UNKNOWN_REFLECT_TYPE:
        errorDesc = tr("Unknown reflect type.");
        break;
    case CCErrorCode::ERROR_DISPLAY_APPLY_FAILED:
        errorDesc = tr("The current settings cannot be applied.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_THEME_NOT_EXIST:
        errorDesc = tr("Theme not exist.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_UNSUPPORTED:
        errorDesc = tr("Unsupported theme type.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_THEME_TYPE_INVALID:
        errorDesc = tr("Invalid theme type.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_1:
    case CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_2:
    case CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_INVALID_3:
        errorDesc = tr("Invalid font type.");
        break;
    case CCErrorCode::ERROR_APPEARANCE_FONT_TYPE_UNSUPPORTED:
        errorDesc = tr("Unsupported font type.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_EXIST_REQUEST_INCOMPLETE:
        errorDesc = tr("An incomplete request already exists.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_CANCELED:
        errorDesc = tr("The request is canceled.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_1:
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_2:
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_3:
    case CCErrorCode::ERROR_BLUETOOTH_REQUEST_REJECTED_4:
        errorDesc = tr("The request is rejected.");
        break;
    case CCErrorCode::ERROR_BLUETOOTH_NOTFOUND_ADAPTOR:
        errorDesc = tr("Not found adapter.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_EXCEED_LIMIT:
        errorDesc = tr("The number of the layout can't exceeds {0}.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_INVALID:
        errorDesc = tr("The layout is invalid.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_ALREADY_EXIST:
        errorDesc = tr("The layout already exist.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_NOT_EXIST:
        errorDesc = tr("The layout is no exist.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_ALREADY_EXIST:
        errorDesc = tr("The layout option already exist.");
        break;
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_OPTION_NOT_EXIST:
        errorDesc = tr("The layout option is no exist.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_SHORTCUT_NOT_EXIST:
        errorDesc = tr("The custom shortcut isn't exist.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_ALREADY_EXIST:
    case CCErrorCode::ERROR_KEYBINDING_SYSTEM_KEYCOMB_ALREADY_EXIST:
        errorDesc = tr("The key combination already exist.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_CUSTOM_KEYCOMB_INVALID:
    case CCErrorCode::ERROR_KEYBINDING_SYSTEM_KEYCOMB_INVALID:
        errorDesc = tr("The key combination is invalid.");
        break;
    case CCErrorCode::ERROR_KEYBINDING_SYSTEM_SHORTCUT_NOT_EXIST:
        errorDesc = tr("The system shortcut isn't exist.");
        break;
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_1:
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_2:
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_3:
    case CCErrorCode::ERROR_POWER_SUPPLY_MODE_UNSUPPORTED_4:
        errorDesc = tr("Unsupported power supply mode.");
        break;
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_1:
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_2:
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_3:
    case CCErrorCode::ERROR_POWER_DEVICE_UNSUPPORTED_4:
        errorDesc = tr("Unsupported power device.");
        break;
    case CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_1:
    case CCErrorCode::ERROR_POWER_UNKNOWN_ACTION_2:
        errorDesc = tr("Unknown power action.");
        break;
    case CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_1:
    case CCErrorCode::ERROR_POWER_EVENT_UNSUPPORTED_2:
        errorDesc = tr("Unsupported power event.");
        break;
    case CCErrorCode::ERROR_POWER_SET_BRIGHTNESS_FAILED:
        errorDesc = tr("Failed to set brightness.");
        break;
    case CCErrorCode::ERROR_SYSTEMINFO_TYPE_INVALID:
        errorDesc = tr("The systeminfo type is invalid.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_NOTFOUND_PROPERTY:
        errorDesc = tr("Not found the property.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_TYPE_MISMATCH:
        errorDesc = tr("The type is mismatch.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_INVALID:
        errorDesc = tr("The property is invalid.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_ONLYREAD:
        errorDesc = tr("The property must not be modified manually.");
        break;
    case CCErrorCode::ERROR_XSETTINGS_PROPERTY_UNSUPPORTED:
        errorDesc = tr("The property is unsupported.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_PEP_EMPTY:
        errorDesc = tr("Arguments invalid.");
        break;
    case CCErrorCode::ERROR_NETWORK_PROXY_MODE_INVALID:
        errorDesc = tr("The network proxy mode is invalid.");
        break;
    case CCErrorCode::ERROR_NETWORK_PROXY_CURRENT_MODE_NOT_MANUAL:
        errorDesc = tr("The current network proxy mode is not manual.");
        break;
    case CCErrorCode::ERROR_NETWORK_PROXY_CURRENT_MODE_NOT_AUTO:
        errorDesc = tr("The current network proxy mode is not auto.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_SPAWN_SYNC_FAILED:
    case CCErrorCode::ERROR_ACCOUNTS_SPAWN_EXIT_STATUS:
    case CCErrorCode::ERROR_ACCOUNTS_USER_UNKNOWN_CALLER_UID_1:
    case CCErrorCode::ERROR_ACCOUNTS_USER_GROUP_NOT_FOUND:
    case CCErrorCode::ERROR_DISPLAY_SAVE_CREATE_FILE_FAILED:
    case CCErrorCode::ERROR_DISPLAY_WRITE_CONF_FILE_FAILED:
    case CCErrorCode::ERROR_DISPLAY_NO_ENABLED_MONITOR:
    case CCErrorCode::ERROR_APPEARANCE_RESET_FONT_FAILED:
    case CCErrorCode::ERROR_AUDIO_SET_SINK_ACTIVE_PORT_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_VOLUME_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_BALANCE_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_FADE_FAILED:
    case CCErrorCode::ERROR_AUDIO_DEVICE_SET_MUTE_FAILED:
    case CCErrorCode::ERROR_AUDIO_STREAM_SET_VOLUME_FAILED:
    case CCErrorCode::ERROR_AUDIO_STREAM_SET_MUTE_FAILED:
    case CCErrorCode::ERROR_NETWORK_PROXY_JSON_FORMAT_FAILED:
    case CCErrorCode::ERROR_NETWORK_PROXY_SET_MODE_FAILED:
    case CCErrorCode::ERROR_NETWORK_PROXY_SET_AUTO_PROXY_URL_FAILED:
    case CCErrorCode::ERROR_NETWORK_PROXY_SET_MANUAL_PROXY_FAILED:
    case CCErrorCode::ERROR_NETWORK_PROXY_GET_MANUAL_PROXY_FAILED:
    case CCErrorCode::ERROR_KEYBOARD_LAYOUT_GET_FAILED:
    case CCErrorCode::ERROR_JSON_READ_EXCEPTION:
    case CCErrorCode::ERROR_JSON_WRITE_EXCEPTION:
    case CCErrorCode::ERROR_JSON_RW_EXCEPTION:
        errorDesc = tr("Internel error.");
        break;
    case CCErrorCode::ERROR_ACCOUNTS_USER_COMMAND_UNKNOWN:
        errorDesc = tr("Unknown error.");
        break;
    default:
        errorDesc = tr("Unknown error.");
        break;
    }

    if (attachErrorCode)
    {
        errorDesc.append(QString(tr(" (error code: 0x%1)")).arg(errorCode, 0, 16));
    }
    return errorDesc;
}

// sonarqube block on

}  // namespace Kiran
