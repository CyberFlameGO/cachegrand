/**
 * Copyright (C) 2018-2022 Daniele Salvatore Albano
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 **/

#include <catch2/catch.hpp>

#include <cstdbool>
#include <memory>

#include <netinet/in.h>

#include "clock.h"
#include "exttypes.h"
#include "spinlock.h"
#include "transaction.h"
#include "transaction_spinlock.h"
#include "data_structures/small_circular_queue/small_circular_queue.h"
#include "data_structures/double_linked_list/double_linked_list.h"
#include "data_structures/hashtable/mcmp/hashtable.h"
#include "config.h"
#include "fiber.h"
#include "worker/worker_stats.h"
#include "worker/worker_context.h"
#include "signal_handler_thread.h"
#include "storage/io/storage_io_common.h"
#include "storage/channel/storage_channel.h"
#include "storage/db/storage_db.h"

#include "program.h"

#include "test-modules-redis-command-fixture.hpp"

#pragma GCC diagnostic ignored "-Wwrite-strings"

TEST_CASE_METHOD(TestModulesRedisCommandFixture, "Redis - command - TTL", "[redis][command][TTL]") {
    SECTION("Non-existing key") {
        REQUIRE(send_recv_resp_command_text_and_validate_recv(
                std::vector<std::string>{"TTL", "a_key"},
                ":-2\r\n"));
    }

    SECTION("Key without expiry") {
        REQUIRE(send_recv_resp_command_text_and_validate_recv(
                std::vector<std::string>{"SET", "a_key", "b_value"},
                "+OK\r\n"));

        REQUIRE(send_recv_resp_command_text_and_validate_recv(
                std::vector<std::string>{"TTL", "a_key"},
                ":-1\r\n"));
    }

    SECTION("Key with 5 second expire") {
        char buffer[32] = { 0 };
        size_t out_buffer_length = 0;
        int64_t unixtime_response;
        size_t expected_length = snprintf(nullptr, 0, ":%d\r\n", 5);

        REQUIRE(send_recv_resp_command_text_and_validate_recv(
                std::vector<std::string>{"SET", "a_key", "b_value", "EX", "5"},
                "+OK\r\n"));

        REQUIRE(send_recv_resp_command_multi_recv(
                std::vector<std::string>{"TTL", "a_key"},
                buffer,
                sizeof(buffer),
                &out_buffer_length,
                expected_length,
                send_recv_resp_command_calculate_multi_recv(expected_length)));

        unixtime_response = strtoll(buffer + 1, nullptr, 10);

        REQUIRE((unixtime_response == 4 || unixtime_response == 5));
    }
}
