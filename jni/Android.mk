# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gikou-$(TARGET_ARCH_ABI)
LOCAL_SRC_FILES := ../src/mate1ply.cc ../src/bench.cc ../src/bitboard.cc ../src/book.cc ../src/cli.cc ../src/cluster.cc ../src/evaluation.cc ../src/extended_board.cc \
                   ../src/gamedb.cc ../src/hand.cc ../src/hash_table.cc ../src/learning.cc ../src/main.cc ../src/mate3.cc \
                   ../src/material.cc ../src/move.cc ../src/move_feature.cc ../src/move_probability.cc ../src/movegen.cc \
                   ../src/movepick.cc ../src/node.cc ../src/notations.cc ../src/position.cc ../src/process.cc ../src/progress.cc ../src/proofpiece.cc \
                   ../src/psq.cc ../src/search.cc ../src/square.cc ../src/swap.cc ../src/synced_printf.cc ../src/task_thread.cc \
                   ../src/thinking.cc ../src/thread.cc ../src/time_control.cc ../src/time_manager.cc ../src/usi.cc ../src/usi_protocol.cc \
                   ../src/zobrist.cc
ARCH_DEF := -DTARGET_ARCH="$(TARGET_ARCH_ABI)"
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
  ARCH_DEF += -DIS_64BIT -mfpu=neon
endif

ifeq ($(TARGET_ARCH_ABI),x86_64)
  ARCH_DEF += -DIS_64BIT
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  ARCH_DEF += -mfpu=neon
endif

LOCAL_CXXFLAGS  := -std=c++11 -O3 -Wall -Wextra -Wcast-qual -fno-exceptions -fno-rtti -pedantic -Wno-long-long  -Wno-error=format-security \
                   -D__STDC_CONSTANT_MACROS -D __STDINT_LIMITS -DMINIMUM $(ARCH_DEF)\

LOCAL_CXXFLAGS += -fPIE
LOCAL_LDFLAGS += -fPIE -pie 
LOCAL_LDLIBS = 

LOCAL_C_INCLUDES := ../ ../src
LOCAL_CPP_FEATURES += exceptions rtti

include $(BUILD_EXECUTABLE)
