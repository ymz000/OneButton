/*

One Button
----------

By: Arko

Base Libraries: Andy Brown - https://github.com/andysworkshop/stm32plus

*/

#include "config/stm32plus.h"
#include "config/usb/device/device.h"
#include "config/timing.h"

using namespace stm32plus;


class OneButton {

  protected:

    /*
     * Definition for the LED. Change to suit your board.
     */

    enum { 
      KEY_IN_PIN = 6,
      KEY_OUT_PIN = 5
    };

    typedef GpioA<DefaultDigitalInputFeature<KEY_IN_PIN>> ButtonInPort;
    typedef GpioA<DefaultDigitalOutputFeature<KEY_OUT_PIN>> ButtonOutPort;


    /*
     * The constants in this structure are used to customise the HID to your
     * requirements.
     */

    struct UsbHidKeyboard {

      enum {

        /*
         * USB Vendor and Product ID. Unfortunately commercial users will probably have to pay
         * the license fee to get an official VID and 64K PIDs with it. For testing and hacking
         * you can just do some research to find an unused VID and use it as you wish.
         */

        VID = 0xCAFE,
        PID = 0xDEAD,



        IN_ENDPOINT_MAX_PACKET_SIZE = 9,   // 1 byte report id + 8-byte report
        OUT_ENDPOINT_MAX_PACKET_SIZE = 2,  // 1 byte report id + 1-byte report

        /*
         * The number of milliamps that our device will use. The maximum you can specify is 510.
         */

        MILLIAMPS = 500,

        /*
         * Additional configuration flags for the device. The available options that can be
         * or'd together are UsbConfigurationFlags::SELF_POWERED and
         * UsbConfigurationFlags::REMOTE_WAKEUP.
         */

        CONFIGURATION_FLAGS = 0,      // we want power from the bus

        /*
         * The language identifier for our strings
         */

        LANGUAGE_ID = 0x0409    // US English.
      };

      /*
       * USB devices support a number of Unicode strings that are used to show information
       * about the device such as the manufacturer, product, serial number and some other
       * stuff that's not usually as visible to the user. You need to define all 5 of them
       * here with the correct byte length. Look ahead to where these are defined to see
       * what the byte lengths will be and then come back here and set them accordingly.
       */

      static const uint8_t ManufacturerString[32];
      static const uint8_t ProductString[22];
      static const uint8_t SerialString[12];
      static const uint8_t ConfigurationString[8];
      static const uint8_t InterfaceString[8];
    };


    /*
     * Member variables for this demo
     */

    volatile bool _deviceConfigured;
    volatile uint32_t _receivedReportTime;
    volatile uint32_t _lastTransmitTime;

  public:

    void run() {

      /*
       * Set up the default values for the member variables
       */

      _deviceConfigured=false;
      _receivedReportTime=UINT32_MAX-1000;
      _lastTransmitTime=0;

      /*
       * Declare the One Button Key port
       */

      ButtonOutPort keyout;
      keyout[KEY_OUT_PIN].reset();
      keyout.setState(0);

      ButtonInPort keyin;
      keyin[KEY_IN_PIN].reset();


      /*
       * Declare the USB custom HID object. This will initialise pins but won't
       * power up the device yet.
       */

      UsbKeyboard<UsbHidKeyboard> usb;


      /*
       * Subscribe to all the events
       */

      usb.UsbRxEventSender.insertSubscriber(UsbRxEventSourceSlot::bind(this,&OneButton::onReceive));
      usb.UsbTxCompleteEventSender.insertSubscriber(UsbTxCompleteEventSourceSlot::bind(this,&OneButton::onTransmitComplete));
      usb.UsbStatusEventSender.insertSubscriber(UsbStatusEventSourceSlot::bind(this,&OneButton::onStatusChange));

      /*
       * Start the peripheral. This will pull up the DP line which is the trigger for the host
       * to start enumeration of this device
       */

      usb.start();

      /*
       * Go into an infinite loop
       */

      for(;;)
      {
        uint8_t _debounce = 0;

        if(keyin.read()==0 && _debounce == 0)
        {
          uint8_t usb_key_report[8] = {2, 0, 5, 0, 0, 0, 0, 0};

          usb.sendReport(usb_key_report,sizeof(usb_key_report));

          MillisecondTimer::delay(10);

          uint8_t usb_key_report_2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

          usb.sendReport(usb_key_report_2,sizeof(usb_key_report_2));
          
          MillisecondTimer::delay(10);

          _debounce = 1;
          MillisecondTimer::delay(200);
        }

        if(keyin.read() == 1)
        {
          _debounce = 0;
        }

      }
    }


    /*
     * Data received from the host
     */

    void onReceive(uint8_t endpointIndex,const uint16_t *data,uint16_t size) {

      // note that the report data is always prefixed with the report id, which is
      // 0x02 in the stm32plus custom HID implementation for reports OUT from the host

      if(endpointIndex==1 && size==2 && memcmp(data,"\x02\x01",size)==0)
        _receivedReportTime=MillisecondTimer::millis();
    }


    /*
     * Finished sending data to the host
     */

    void onTransmitComplete(uint8_t /* endpointIndex */,uint16_t /* size */) {
      // ACK received from the host
    }


    /*
     * Device status change event
     */

    void onStatusChange(UsbStatusType newStatus) {

      switch(newStatus) {

        case UsbStatusType::STATE_CONFIGURED:
        _deviceConfigured=true;
        _lastTransmitTime=MillisecondTimer::millis()+5000;    // 5 second delay before starting to send
        break;

        case UsbStatusType::STATE_DEFAULT:
        case UsbStatusType::STATE_ADDRESSED:
        case UsbStatusType::STATE_SUSPENDED:
          _deviceConfigured=false;
          break;

        default:
          break;
      }
    }
};


/*
 * These are the USB device strings in the format required for a USB string descriptor.
 * To change these to suit your device you need only change the unicode string in the
 * last line of each definition to suit your device. Then count up the bytes required for
 * the complete descriptor and go back and insert that byte count in the array declaration
 * in the configuration class.
 */

const uint8_t OneButton::UsbHidKeyboard::ManufacturerString[sizeof(OneButton::UsbHidKeyboard::ManufacturerString)]={
  sizeof(OneButton::UsbHidKeyboard::ManufacturerString),
  USB_DESC_TYPE_STRING,
  'O',0,'N',0,'E',0,' ',0,'B',0,'U',0,'T',0,'T',0,'O',0,'N',0
};

const uint8_t OneButton::UsbHidKeyboard::ProductString[sizeof(OneButton::UsbHidKeyboard::ProductString)]={
  sizeof(OneButton::UsbHidKeyboard::ProductString),
  USB_DESC_TYPE_STRING,
  'O',0,'N',0,'E',0,' ',0,'B',0,'U',0,'T',0,'T',0,'O',0,'N',0
};

const uint8_t OneButton::UsbHidKeyboard::SerialString[sizeof(OneButton::UsbHidKeyboard::SerialString)]={
  sizeof(OneButton::UsbHidKeyboard::SerialString),
  USB_DESC_TYPE_STRING,
  '1',0,'.',0,'0',0,'.',0,'0',0
};

const uint8_t OneButton::UsbHidKeyboard::ConfigurationString[sizeof(OneButton::UsbHidKeyboard::ConfigurationString)]={
  sizeof(OneButton::UsbHidKeyboard::ConfigurationString),
  USB_DESC_TYPE_STRING,
  'c',0,'f',0,'g',0
};

const uint8_t OneButton::UsbHidKeyboard::InterfaceString[sizeof(OneButton::UsbHidKeyboard::InterfaceString)]={
  sizeof(OneButton::UsbHidKeyboard::InterfaceString),
  USB_DESC_TYPE_STRING,
  'i',0,'t',0,'f',0
};

/*
 * Main entry point
 */

int main() {

  Nvic::initialise();

  // set up SysTick at 1ms resolution
  MillisecondTimer::initialise();

  OneButton hid;
  hid.run();

  // not reached
  return 0;
}
