#ifndef __ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_CRS_H__
#define __ZEPHYR_INCLUDE_BLUETOOTH_SERVICES_CRS_H__

#include <stdint.h>
#include <zephyr/sys/slist.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Server shall restart the accumulation of energy expended from zero
 */
#define BT_CRS_CONTROL_POINT_RESET_ENERGY_EXPANDED_REQ 0x01

/** @brief Heart rate service callback structure */
struct bt_crs_cb {
    /** @brief Heart rate notifications changed
     *
     * @param enabled Flag that is true if notifications were enabled, false
     *                if they were disabled.
     */
    void (*ntf_changed)(bool enabled);

    /**
     * @brief Heart rate control point write callback
     *
     * @note if Server supports the Energy Expended feature then application
     * shall implement and support @ref BT_HRS_CONTROL_POINT_RESET_ENERGY_EXPANDED_REQ
     * request code
     *
     * @param request control point request code
     *
     * @return 0 on successful handling of control point request
     * @return -ENOTSUP if not supported. It can be used to pass handling to other
     *         listeners in case of multiple listeners
     * @return other negative error codes will result in immediate error response
     */
    int (*ctrl_point_write)(uint8_t request);

    /** Internal member to form a list of callbacks */
    sys_snode_t _node;
};

/** @brief Heart rate service callback register
 *
 * This function will register callbacks that will be called in
 * certain events related to Heart rate service.
 *
 * @param cb Pointer to callbacks structure. Must point to memory that remains valid
 * until unregistered.
 *
 * @return 0 on success
 * @return -EINVAL in case @p cb is NULL
 */
int bt_crs_cb_register(struct bt_crs_cb* cb);

/** @brief Heart rate service callback unregister
 *
 * This function will unregister callback from Heart rate service.
 *
 * @param cb Pointer to callbacks structure
 *
 * @return 0 on success
 * @return -EINVAL in case @p cb is NULL
 * @return -ENOENT in case the @p cb was not found in registered callbacks
 */
int bt_crs_cb_unregister(struct bt_crs_cb* cb);

/** @brief Notify heart rate measurement.
 *
 * This will send a GATT notification to all current subscribers.
 *
 *  @param heartrate The heartrate measurement in beats per minute.
 *
 *  @return Zero in case of success and error code in case of error.
 */
int bt_crs_notify(uint8_t* cr_data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
