#pragma once

#include "opendbc/safety/safety_declarations.h"

static void body_rx_hook(const CANPacket_t *msg) {
  if (GET_ADDR(msg) == 0x201U) {
    controls_allowed = true;
  }
}

static bool body_tx_hook(const CANPacket_t *msg) {
  bool tx = true;
  int addr = GET_ADDR(msg);
  int len = GET_LEN(msg);

  if (!controls_allowed && (addr != 0x1)) {
    tx = false;
  }

  // Allow going into CAN flashing mode even if controls are not allowed
  bool flash_msg = (addr == 0x250) && (len == 8);
  if (!controls_allowed && (GET_BYTES(msg, 0, 4) == 0xdeadfaceU) && (GET_BYTES(msg, 4, 4) == 0x0ab00b1eU) && flash_msg) {
    tx = true;
  }

  return tx;
}

static safety_config body_init(uint16_t param) {
  static RxCheck body_rx_checks[] = {
    {.msg = {{0x201, 0, 8, .ignore_checksum = true, .ignore_counter = true, .ignore_quality_flag = true, .frequency = 100U}, { 0 }, { 0 }}},
  };

  static const CanMsg BODY_TX_MSGS[] = {{0x250, 0, 8, .check_relay = false}, {0x250, 0, 6, .check_relay = false}, {0x251, 0, 5, .check_relay = false},  // body
                                        {0x1, 0, 8, .check_relay = false}};  // CAN flasher

  UNUSED(param);
  safety_config ret = BUILD_SAFETY_CFG(body_rx_checks, BODY_TX_MSGS);
  ret.disable_forwarding = true;
  return ret;
}

const safety_hooks body_hooks = {
  .init = body_init,
  .rx = body_rx_hook,
  .tx = body_tx_hook,
};
