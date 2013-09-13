#ifndef ITDSRC_MAC_H
#define ITDSRC_MAC_H

/** \file itdsrc_mac.h
 *  \brief Interface for the MAC of the ITDSRC communications device.
 *
 *  This file declares the interface used to interact with the itdsrc
 *  communications device according to existing standards.
 *
 *  Besides the declared functions and structures, there are three callbacks
 *  that must be registered with the ma_init function.  Each of the callbacks is
 *  associated with the corresponding "indication" event. As an example,
 *  ma_unitdata_indication_cb is associated with the MA-UNITDATA.indication
 *  event. All the callbacks receive a pointer to the corresponding event
 *  structure and any allocated resources are automatically destroyed after the
 *  callback returns. The callbacks do not need to be reentrant.
 *
 *  To use the device, start by issuing an ma_init() and registering the callbacks.
 *  Call ma_*_request as needed, the callbacks will be automatically called
 *  as events occur. To finish, issue an ma_stop().
 */

#include <stdint.h> /* uint8_t type */

/** Opaque structure that represents a physical device. */
struct ma_dev;

/** Parameters for the MA-UNITDATA.indication primitive */
struct ma_unitdata_indication {
	/** Source address */
	uint8_t source_address[6];

	/** Destination address */
	uint8_t destination_address[6];

	/** Pointer to the load */
	uint8_t *data;

	/** Load length, in bytes */
	int data_length;

	/** Reception status (must describe possible values) */
	uint8_t reception_status;

	/** Priority */
	uint8_t priority;

	/** Service class */
	uint8_t service_class;
};

/** Parameters for the MA-UNITDATA-STATUS.indication primitive */
struct ma_unitdata_status_indication {
	/** Source address */
	uint8_t source_address[6];

	/** Destination address */
	uint8_t destination_address[6];

	/** Transmission status (must describe possible values) */
	uint8_t transmission_status;

	/** Provided priority */
	uint8_t provided_priority;

	/** Provided service class */
	uint8_t provided_service_class;
};

/** Parameters for the MA-UNITDATAX-STATUS.indication primitive */
struct ma_unitdatax_status_indication {
	/** Source address */
	uint8_t source_address[6];

	/** Destination address */
	uint8_t destination_address[6];

	/** Transmission status (must describe possible values) */
	uint8_t transmission_status;

	/** Provided priority */
	uint8_t provided_priority;

	/** Provided service class */
	uint8_t provided_service_class;
};

/** Initialize the device and register the callbacks for the indications.
 *
 *  @param dev_id
 *    device id to attach to
 *  @param dev
 *    the caller will be attached to this device
 *  @param ma_unitdata_indication_cb
 *    pointer to the callback function relative to the MA-UNITDATA.indication
 *  @param ma_unitdata_status_indication_cb
 *    pointer to the callback function relative to the MA-UNITDATA-STATUS.indication
 *  @param ma_unitdatax_status_indication_cb
 *    pointer to the callback function relative to the MA-UNITDATAX-STATUS.indication
 *
 *  @return 0 : success
 *  @return -1 : failure
 */
int ma_init(
	int dev_id,
	struct ma_dev **dev,
	void (*ma_unitdata_indication_cb)(const struct ma_unitdata_indication *),
	void (*ma_unitdata_status_indication_cb)(const struct ma_unitdata_status_indication *),
	void (*ma_unitdatax_status_indication_cb)(const struct ma_unitdatax_status_indication *)
);

/** Release allocated resources and close the device.
 *  
 *  @param dev
 *    device to operate on
 */
void ma_stop(struct ma_dev *dev);

/** Issue an MA-UNITDATA.request.
 *
 *  After successfully issuing a request, ma_unitdata_status_indication_cb will
 *  be called exactly once with the correspondent ma_unitdata_status_indication.
 *  Successfull isuing of a request does not mean that the request will be
 *  granted, only that it will be analized for granting. It may still fail,
 *  condition that is reported via the ma_unitdata_status_indication_cb.
 *
 *  @param dev
 *    device to operate on
 *  @param source_address
 *    Source address
 *  @param destination_address
 *    Destination address
 *  @param data
 *    Pointer to the load
 *  @param data_length
 *    Data vector length, in bytes
 *  @param priority
 *    Priority
 *  @param service_class
 *    Service class
 *
 *  @return 0 : success
 *  @return -1 : failure
 */
int ma_unitdata_request(
	struct ma_dev *dev,
	const uint8_t source_address[6],
	const uint8_t destination_address[6],
	const uint8_t *data,
	int data_length,
	uint8_t priority,
	uint8_t service_class
);

/** Issue an MA-UNITDATAX.request.
 *
 *  After successfully issuing a request, ma_unitdatax_status_indication_cb will
 *  be called exactly once with the correspondent
 *  ma_unitdatax_status_indication. Successfull isuing of a request does not
 *  mean that the request will be granted, only that it will be analized for
 *  granting. It may still fail, condition that is reported via the
 *  ma_unitdatax_status_indication_cb.
 *
 *  @param dev
 *    device to operate on
 *  @param source_address
 *    Source address
 *  @param destination_address
 *    Destination address
 *  @param data
 *    Pointer to the load
 *  @param data_length
 *    Data vector length, in bytes
 *  @param priority
 *    Priotity
 *  @param service_class
 *    Service class
 *  @param channel_id
 *    Channel id (semantic to be defined)
 *  @param data_rate
 *    Data rate (semantic to be defined)
 *  @param power_level
 *    Transmission power level (semantic to be defined)
 *  @param expiry_time
 *    Expiry time (semantic to be defined)
 *
 *  @return 0 : success
 *  @return -1 : failure
 */
int ma_unitdatax_request(
	struct ma_dev *dev,
	const uint8_t source_address[6],
	const uint8_t destination_address[6],
	const uint8_t *data,
	int data_length,
	uint8_t priority,
	uint8_t service_class,
	uint8_t channel_id,
	uint8_t data_rate,
	uint8_t power_level,
	uint8_t expiry_time
);

#endif /* ITDSRC_MAC_H */
