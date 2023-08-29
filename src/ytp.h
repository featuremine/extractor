/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file ytp.h
 * @author Federico Ravchina
 * @date 20 Dec 2021
 * @brief File contains C definitions of the ytp wrapper
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/frame.h"
#include "ytp/api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ytp_msg_decoded {
  uint64_t time;
  fm_frame_t *frame;
};

struct ytp_sequence_wrapper {
  shared_sequence *sequence;
};

struct ytp_stream_wrapper {
  shared_sequence *sequence;
  ytp_peer_t peer;
  ytp_channel_t channel;
};

struct ytp_channel_wrapper {
  shared_sequence *sequence;
  ytp_channel_t channel;
};

struct ytp_peer_wrapper {
  shared_sequence *sequence;
  ytp_peer_t peer;
};

#ifdef __cplusplus
}
#endif
