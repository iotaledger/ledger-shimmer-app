#include "ux.h"
#include "glyphs.h"

#include "ui_common.h"
#include "flow_user_confirm.h"
#include "flow_user_confirm_new_address.h"
#include "abstraction.h"

extern flowdata_t flow_data;

// gcc doesn't know this and ledger's SDK cannot be compiled with Werror!
//#pragma GCC diagnostic error "-Werror"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wmissing-prototypes"

static void cb_address_preinit();

static void cb_accept();

// clang-format off

static UX_STEP_NOCB_INIT(
    ux_step_new_address,
    bn_paging,
    cb_address_preinit(),
    {
        "Receive Address", (const char*) flow_data.data
    }
);

static UX_STEP_NOCB_INIT(
    ux_step_new_remainder,
    bn_paging,
    cb_address_preinit(),
    {
        "New Remainder", (const char*) flow_data.data
    }
);

static UX_STEP_CB(
    ux_step_ok,
    pb,
    cb_accept(),
    {
        &C_x_icon_check,
        "Ok"
    }
);

static UX_FLOW(
    ux_flow_new_address,
    &ux_step_new_address,
    &ux_step_ok,
    FLOW_LOOP
);

static UX_FLOW(
    ux_flow_new_remainder,
    &ux_step_new_remainder,
    &ux_step_ok,
    FLOW_LOOP
);

// clang-format on

static void cb_address_preinit()
{
    // clear buffer
    memset(flow_data.data, 0, sizeof(flow_data.data));

    // generate bech32 address including the address_type
    // since the struct is packed, the address follows directly the address_type
    address_encode_bech32(flow_data.api->data.buffer, flow_data.data,
                          sizeof(flow_data.data));
}

static void cb_accept()
{
    if (flow_data.accept_cb) {
        flow_data.accept_cb();
    }
    flow_stop();
}

void flow_start_new_address(const API_CTX *api, accept_cb_t accept_cb,
                            timeout_cb_t timeout_cb)
{
    flow_start_user_confirm(api, accept_cb, 0, timeout_cb);

    if (flow_data.api->bip32_path[BIP32_CHANGE_INDEX] & 0x1) {
        ux_flow_init(0, ux_flow_new_remainder, &ux_step_new_remainder);
    }
    else {
        ux_flow_init(0, ux_flow_new_address, &ux_step_new_address);
    }
}
