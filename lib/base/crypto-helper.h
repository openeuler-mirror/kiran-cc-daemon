/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd. 
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

#include <cstdint>
#include <string>

namespace Kiran
{
#define DEFAULT_DES_KEY "ccdaemon"

class CryptoHelper
{
public:
    CryptoHelper();
    virtual ~CryptoHelper();

    // md5
    static std::string md5(const std::string &message);
    static std::string md5_file(const std::string &filename);

    // base64加密和解密
    static std::string base64_encrypt(const std::string &message);
    static std::string base64_decrypt(const std::string &message);

    // 生成rsa公钥和私钥对
    static void generate_rsa_key(uint32_t key_length,
                                 std::string &private_key,
                                 std::string &public_key);

    // rsa加密和解密
    static std::string rsa_encrypt(const std::string &public_key, const std::string &message);
    static std::string rsa_decrypt(const std::string &private_key, const std::string &ciphertext);

    // rsa签名
    static bool rsa_sign_file(const std::string &private_filename,
                              const std::string &message_filename,
                              const std::string &signature_filename);
    static bool rsa_verify_file(const std::string &public_filename,
                                const std::string &message_filename,
                                const std::string &signature_filename);

    // des加密和解密
    static std::string des_encrypt(const std::string &message, const std::string &key = DEFAULT_DES_KEY);
    static std::string des_decrypt(const std::string &message, const std::string &key = DEFAULT_DES_KEY);
};
}  // namespace Kiran
