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

#include <gtest/gtest.h>
#include "lib/base/crypto-helper.h"

TEST(RSATest, EncryptAndDecrypt)
{
    std::string raw_text("Hello world!");

    std::string rsa_private_key;
    std::string rsa_public_key;

    Kiran::CryptoHelper::generate_rsa_key(512, rsa_private_key, rsa_public_key);
    auto encrypted_text = Kiran::CryptoHelper::rsa_encrypt(rsa_public_key, raw_text);
    auto decrypted_text = Kiran::CryptoHelper::rsa_decrypt(rsa_private_key, encrypted_text);

    // printf("private: %s, public: %s, encrypted_text: %s\n",
    //        rsa_private_key.c_str(),
    //        rsa_public_key.c_str(),
    //        encrypted_text.c_str());

    ASSERT_STREQ(raw_text.c_str(), decrypted_text.c_str());
}

TEST(DESTest, EncryptAndDecrypt)
{
    Kiran::CryptoHelper crypto_helper;
    std::string raw_text("aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbccccccccccccccccccccccccddddddddddddddd");

    auto encrypted_text = Kiran::CryptoHelper::des_encrypt(raw_text);
    auto decrypted_text = Kiran::CryptoHelper::des_decrypt(encrypted_text);
    ASSERT_STREQ(raw_text.c_str(), decrypted_text.c_str());
}