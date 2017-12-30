// Copyright (c) 2014-2017, The Monero Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include "include_base_utils.h"

using namespace epee;

#include "checkpoints.h"

#include "common/dns_utils.h"
#include "include_base_utils.h"
#include "storages/portable_storage_template_helper.h" // epee json include
#include <sstream>
#include <random>

#undef MONERO_DEFAULT_LOG_CATEGORY
#define MONERO_DEFAULT_LOG_CATEGORY "checkpoints"

namespace cryptonote
{
  //---------------------------------------------------------------------------
  checkpoints::checkpoints()
  {
  }
  //---------------------------------------------------------------------------
  bool checkpoints::add_checkpoint(uint64_t height, const std::string& hash_str)
  {
    crypto::hash h = crypto::null_hash;
    bool r = epee::string_tools::parse_tpod_from_hex_string(hash_str, h);
    CHECK_AND_ASSERT_MES(r, false, "Failed to parse checkpoint hash string into binary representation!");

    // return false if adding at a height we already have AND the hash is different
    if (m_points.count(height))
    {
      CHECK_AND_ASSERT_MES(h == m_points[height], false, "Checkpoint at given height already exists, and hash for new checkpoint was different!");
    }
    m_points[height] = h;
    return true;
  }
  //---------------------------------------------------------------------------
  bool checkpoints::is_in_checkpoint_zone(uint64_t height) const
  {
    return !m_points.empty() && (height <= (--m_points.end())->first);
  }
  //---------------------------------------------------------------------------
  bool checkpoints::check_block(uint64_t height, const crypto::hash& h, bool& is_a_checkpoint) const
  {
    auto it = m_points.find(height);
    is_a_checkpoint = it != m_points.end();
    if(!is_a_checkpoint)
      return true;

    if(it->second == h)
    {
      MINFO("CHECKPOINT PASSED FOR HEIGHT " << height << " " << h);
      return true;
    }else
    {
      MWARNING("CHECKPOINT FAILED FOR HEIGHT " << height << ". EXPECTED HASH: " << it->second << ", FETCHED HASH: " << h);
      return false;
    }
  }
  //---------------------------------------------------------------------------
  bool checkpoints::check_block(uint64_t height, const crypto::hash& h) const
  {
    bool ignored;
    return check_block(height, h, ignored);
  }
  //---------------------------------------------------------------------------
  //FIXME: is this the desired behavior?
  bool checkpoints::is_alternative_block_allowed(uint64_t blockchain_height, uint64_t block_height) const
  {
    if (0 == block_height)
      return false;

    auto it = m_points.upper_bound(blockchain_height);
    // Is blockchain_height before the first checkpoint?
    if (it == m_points.begin())
      return true;

    --it;
    uint64_t checkpoint_height = it->first;
    return checkpoint_height < block_height;
  }
  //---------------------------------------------------------------------------
  uint64_t checkpoints::get_max_height() const
  {
    std::map< uint64_t, crypto::hash >::const_iterator highest = 
        std::max_element( m_points.begin(), m_points.end(),
                         ( boost::bind(&std::map< uint64_t, crypto::hash >::value_type::first, _1) < 
                           boost::bind(&std::map< uint64_t, crypto::hash >::value_type::first, _2 ) ) );
    return highest->first;
  }
  //---------------------------------------------------------------------------
  const std::map<uint64_t, crypto::hash>& checkpoints::get_points() const
  {
    return m_points;
  }

  bool checkpoints::check_for_conflicts(const checkpoints& other) const
  {
    for (auto& pt : other.get_points())
    {
      if (m_points.count(pt.first))
      {
        CHECK_AND_ASSERT_MES(pt.second == m_points.at(pt.first), false, "Checkpoint at given height already exists, and hash for new checkpoint was different!");
      }
    }
    return true;
  }

  bool checkpoints::init_default_checkpoints(bool testnet)
  {
    if (testnet)
    {
      ADD_CHECKPOINT(0,     "48ca7cd3c8de5b6a4d53d2861fbdaedca141553559f9be9520068053cda8430b");
      ADD_CHECKPOINT(1000000, "46b690b710a07ea051bc4a6b6842ac37be691089c0f7758cfeec4d5fc0b4a258");
      return true;
    }
    ADD_CHECKPOINT(1, "aeeff201e743104731977b820d40a8d95b1f07a4472c019e603271d0cb0f047a");
    ADD_CHECKPOINT(100, "d33ee17506e4bf8e588c59717baaf8233ffe6f0783e93e5b95d9f13dde326a61");
    ADD_CHECKPOINT(1000, "7656bae1cfede8dc962a603849e210736d1366a97ed3f6543b223160a2568f75");
    ADD_CHECKPOINT(1200, "12a70b65ff083e68a0260724596208367be88420089b546df07a510fa5978fc3");
    ADD_CHECKPOINT(1300, "ed0e00d6b456ba3c8ae06488f84e3e04a85b660564cfbd5e1dcb9c4e53714bd5");
    ADD_CHECKPOINT(1327, "d5d6c64d78e48f2ede80d6efcca81cc00b4e0bce4f7ddc376e3b936d3b2d63a1");
    ADD_CHECKPOINT(4577, "7d37a5d91270cb04294bdb6e09a6ce25131f68441d1307c7f3ec97c978f66bfa");
    ADD_CHECKPOINT(6014, "5ec2fef6918247e6b6cd17a256b379e60c2af67058a15d7d808d4774b5c7f6ad");
    ADD_CHECKPOINT(6112, "9d47805b39787cb4dfac49215f6a1729a39bc4dd8996a81b84598fe0aecaece4");
    ADD_CHECKPOINT(6368, "3129cf2d5cc8792aefaa5c2a9f77986a384bfb73381cb14d9dc6867cd1df9d92");
    ADD_CHECKPOINT(6556, "8a6466d57f1e95b543d14ac41783a94a83612b974303e5640b8da6c14d85d3bc");
    ADD_CHECKPOINT(7205, "3abe25b8ff8abaa2c628f79e73daff4d1c9ff7afb054ef8038717d4d346807f3");
    ADD_CHECKPOINT(8281, "a92baf6aaa903c62696d63f2f193000b482c6e63f9d58bdc0316d2c29ea87289");
    ADD_CHECKPOINT(8305, "fca0a4eaadab69324d948eb3086509de72ceefb861d28a27e983b089cdfc9add");
    ADD_CHECKPOINT(9109, "0860093f1cb43fd08e84fcad1a74b07746b2c810c7db81aa2692496a7b3d9507");
    ADD_CHECKPOINT(10689, "0f148437a494dc05a8402d150b6a6ebeffcf0a2d176de64349137a51566d4456");
    ADD_CHECKPOINT(11415, "62e45e4c5236dd547fd156600d4f41aefc8155d6bb98121cbb506b878257b246");
    ADD_CHECKPOINT(12029, "77254e38b4b16e0d65ed7cc9d2a6c2a557b9c1e20a9020c349cb10774bad4696");
    ADD_CHECKPOINT(12386, "7b485d5c82555d73dddbae201486b78fe1b4270ff0e52ac43d755189f6b8726d");
    // ADD_CHECKPOINT(22231, "7cb10e29d67e1c069e6e11b17d30b809724255fee2f6868dc14cfc6ed44dfb25");
    // ADD_CHECKPOINT(29556, "53c484a8ed91e4da621bb2fa88106dbde426fe90d7ef07b9c1e5127fb6f3a7f6");
    // ADD_CHECKPOINT(50000, "0fe8758ab06a8b9cb35b7328fd4f757af530a5d37759f9d3e421023231f7b31c");
    // ADD_CHECKPOINT(80000, "a62dcd7b536f22e003ebae8726e9e7276f63d594e264b6f0cd7aab27b66e75e3");
    // ADD_CHECKPOINT(202612, "bbd604d2ba11ba27935e006ed39c9bfdd99b76bf4a50654bc1e1e61217962698");
    // ADD_CHECKPOINT(202613, "e2aa337e78df1f98f462b3b1e560c6b914dec47b610698b7b7d1e3e86b6197c2");
    // ADD_CHECKPOINT(202614, "c29e3dc37d8da3e72e506e31a213a58771b24450144305bcba9e70fa4d6ea6fb");
    // ADD_CHECKPOINT(205000, "5d3d7a26e6dc7535e34f03def711daa8c263785f73ec1fadef8a45880fde8063");
    // ADD_CHECKPOINT(220000, "9613f455933c00e3e33ac315cc6b455ee8aa0c567163836858c2d9caff111553");
    // ADD_CHECKPOINT(230300, "bae7a80c46859db355556e3a9204a337ae8f24309926a1312323fdecf1920e61");
    // ADD_CHECKPOINT(230700, "93e631240ceac831da1aebfc5dac8f722c430463024763ebafa888796ceaeedf");
    // ADD_CHECKPOINT(231350, "b5add137199b820e1ea26640e5c3e121fd85faa86a1e39cf7e6cc097bdeb1131");
    // ADD_CHECKPOINT(232150, "955de8e6b6508af2c24f7334f97beeea651d78e9ade3ab18fec3763be3201aa8");
    // ADD_CHECKPOINT(249380, "654fb0a81ce3e5caf7e3264a70f447d4bd07586c08fa50f6638cc54da0a52b2d");
    // ADD_CHECKPOINT(460000, "75037a7aed3e765db96c75bcf908f59d690a5f3390baebb9edeafd336a1c4831");
    // ADD_CHECKPOINT(500000, "2428f0dbe49796be05ed81b347f53e1f7f44aed0abf641446ec2b94cae066b02");
    // ADD_CHECKPOINT(600000, "f5828ebf7d7d1cb61762c4dfe3ccf4ecab2e1aad23e8113668d981713b7a54c5");
    // ADD_CHECKPOINT(700000, "12be9b3d210b93f574d2526abb9c1ab2a881b479131fd0d4f7dac93875f503cd");
    // ADD_CHECKPOINT(825000, "56503f9ad766774b575be3aff73245e9d159be88132c93d1754764f28da2ff60");
    // ADD_CHECKPOINT(900000, "d9958d0e7dcf91a5a7b11de225927bf7efc6eb26240315ce12372be902cc1337");
    // ADD_CHECKPOINT(913193, "5292d5d56f6ba4de33a58d9a34d263e2cb3c6fee0aed2286fd4ac7f36d53c85f");
    // ADD_CHECKPOINT(1000000, "a886ef5149902d8342475fee9bb296341b891ac67c4842f47a833f23c00ed721");
    // ADD_CHECKPOINT(1100000, "3fd720c5c8b3072fc1ccda922dec1ef25f9ed88a1e6ad4103d0fe00b180a5903");
    // ADD_CHECKPOINT(1150000, "1dd16f626d18e1e988490dfd06de5920e22629c972c58b4d8daddea0038627b2");
    // ADD_CHECKPOINT(1200000, "fa7d13a90850882060479d100141ff84286599ae39c3277c8ea784393f882d1f");
    // ADD_CHECKPOINT(1300000, "31b34272343a44a9f4ac7de7a8fcf3b7d8a3124d7d6870affd510d2f37e74cd0");
    // ADD_CHECKPOINT(1390000, "a8f5649dd4ded60eedab475f2bec8c934681c07e3cf640e9be0617554f13ff6c");
    

    return true;
  }

  bool checkpoints::load_checkpoints_from_json(const std::string json_hashfile_fullpath)
  {
    boost::system::error_code errcode;
    if (! (boost::filesystem::exists(json_hashfile_fullpath, errcode)))
    {
      LOG_PRINT_L1("Blockchain checkpoints file not found");
      return true;
    }

    LOG_PRINT_L1("Adding checkpoints from blockchain hashfile");

    uint64_t prev_max_height = get_max_height();
    LOG_PRINT_L1("Hard-coded max checkpoint height is " << prev_max_height);
    t_hash_json hashes;
    epee::serialization::load_t_from_json_file(hashes, json_hashfile_fullpath);
    for (std::vector<t_hashline>::const_iterator it = hashes.hashlines.begin(); it != hashes.hashlines.end(); )
    {
      uint64_t height;
      height = it->height;
      if (height <= prev_max_height) {
	LOG_PRINT_L1("ignoring checkpoint height " << height);
      } else {
	std::string blockhash = it->hash;
	LOG_PRINT_L1("Adding checkpoint height " << height << ", hash=" << blockhash);
	ADD_CHECKPOINT(height, blockhash);
      }
      ++it;
    }

    return true;
  }

  bool checkpoints::load_checkpoints_from_dns(bool testnet)
  {
    std::vector<std::string> records;

    // All four MoneroPulse domains have DNSSEC on and valid
    static const std::vector<std::string> dns_urls = { "checkpoints.moneropulse.se"
						     , "checkpoints.moneropulse.org"
						     , "checkpoints.moneropulse.net"
						     , "checkpoints.moneropulse.co"
    };

    static const std::vector<std::string> testnet_dns_urls = { "testpoints.moneropulse.se"
							     , "testpoints.moneropulse.org"
							     , "testpoints.moneropulse.net"
							     , "testpoints.moneropulse.co"
    };

    if (!tools::dns_utils::load_txt_records_from_dns(records, testnet ? testnet_dns_urls : dns_urls))
      return true; // why true ?

    for (const auto& record : records)
    {
      auto pos = record.find(":");
      if (pos != std::string::npos)
      {
        uint64_t height;
        crypto::hash hash;

        // parse the first part as uint64_t,
        // if this fails move on to the next record
        std::stringstream ss(record.substr(0, pos));
        if (!(ss >> height))
        {
    continue;
        }

        // parse the second part as crypto::hash,
        // if this fails move on to the next record
        std::string hashStr = record.substr(pos + 1);
        if (!epee::string_tools::parse_tpod_from_hex_string(hashStr, hash))
        {
    continue;
        }

        ADD_CHECKPOINT(height, hashStr);
      }
    }
    return true;
  }

  bool checkpoints::load_new_checkpoints(const std::string json_hashfile_fullpath, bool testnet, bool dns)
  {
    bool result;

    result = load_checkpoints_from_json(json_hashfile_fullpath);
    if (dns)
    {
      result &= load_checkpoints_from_dns(testnet);
    }

    return result;
  }
}
