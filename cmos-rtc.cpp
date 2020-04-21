/*
 * CMOS Real-time Clock
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (3)
 */

/*
 * STUDENT NUMBER: s2032224
 */
#include <infos/drivers/timer/rtc.h>
#include <infos/util/lock.h>
#include <arch/x86/pio.h>

using namespace infos::drivers;
using namespace infos::drivers::timer;
using namespace infos::util;
using namespace infos::arch::x86;

class CMOSRTC : public RTC {
public:
	static const DeviceClass CMOSRTCDeviceClass;

	const DeviceClass& device_class() const override
	{
		return CMOSRTCDeviceClass;
	}

	RTCTimePoint get_current_timepoint()
	{
		UniqueIRQLock l;
		while (get_data(0xA, 7) != 0); // update not in progress
		return RTCTimePoint{.seconds = get_data(0x00), .minutes = get_data(0x02), .hours = get_data(0x04), .day_of_month = get_data(0x07), .month = get_data(0x08), .year = get_data(0x09), };
	}

	uint8_t get_data(int reg)
	{
		__outb(0x70, reg); // activate the register
		return __inb(0x71); // read the date from the activated register
	}

	uint8_t get_data(int reg, int bit)
	{
		return (get_data(reg) >> bit) & 1; // get the bit value of the data from a particular register
	}

	/**
	 * Interrogates the RTC to read the current date & time.
	 * @param tp Populates the tp structure with the current data & time, as
	 * given by the CMOS RTC device.
	 */
	void read_timepoint(RTCTimePoint& tp) override
	{
		tp = get_current_timepoint();
		RTCTimePoint prev = tp;
		do {
			prev = tp;
			tp = get_current_timepoint();
		} while (!((tp.seconds == prev.seconds) && (tp.minutes == prev.minutes) && (tp.hours == prev.hours) && (tp.day_of_month == prev.day_of_month) && (tp.month == prev.month) && (tp.year == prev.year) && true));

		// convert from BCD if necessary
		UniqueIRQLock l;
		if (get_data(0xB, 2) == 0) {
			tp.seconds = ((tp.seconds >> 4) * 10) + (tp.seconds & 0xF);
			tp.minutes = ((tp.minutes >> 4) * 10) + (tp.minutes & 0xF);
			tp.hours = ((tp.hours >> 4) * 10) + (tp.hours & 0xF);
			tp.day_of_month = ((tp.day_of_month >> 4) * 10) + (tp.day_of_month & 0xF);
			tp.month = ((tp.month >> 4) * 10) + (tp.month & 0xF);
			tp.year = ((tp.year >> 4) * 10) + (tp.year & 0xF);
		}

		// convert from 12 hr format if necessary
		if ((get_data(0xB, 1) == 0) && (tp.hours & 0x80)) {
			tp.hours = ((tp.hours & 0x7F) + 12) % 24;
		}
	}
};

const DeviceClass CMOSRTC::CMOSRTCDeviceClass(RTC::RTCDeviceClass, "cmos-rtc");

RegisterDevice(CMOSRTC);
