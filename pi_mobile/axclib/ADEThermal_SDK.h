#ifndef ADETHERMAL_H
#define ADETHERMAL_H

#include <inttypes.h>

/* API result code */
typedef enum _E_ADETHERMAL_RESULT
{
    ADETHERMAL_RESULT_SUCCEED = 0,
    ADETHERMAL_RESULT_SDK_NOT_READY = 1,
    ADETHERMAL_RESULT_INVALID_PARAMETER = 2,
    ADETHERMAL_RESULT_NO_ENOUGH_MEMORY = 3,
    ADETHERMAL_RESULT_NOT_SUPPORT = 4,
    ADETHERMAL_RESULT_FAILED_OPEN_SPI = 5,
    ADETHERMAL_RESULT_FAILED_CONFIG_SPI = 6,
    ADETHERMAL_RESULT_FAILED_OPEN_I2C = 7,
    ADETHERMAL_RESULT_FAILED_CONFIG_I2C = 8,
    ADETHERMAL_RESULT_FAILED_IOCTL = 9,
    ADETHERMAL_RESULT_FAILED_CREATE_THREAD = 10,
    ADETHERMAL_RESULT_DEVICE_NOT_OPENED = 11,
    ADETHERMAL_RESULT_NO_MORE_DATA = 12,
    ADETHERMAL_RESULT_FAILED_OPEN_UART = 13,
} E_ADETHERMAL_RESULT;

/* Thermal-Module config */
typedef struct _T_ADETHERMAL_CONFIG
{
    uint32_t config_size;     /* size of this-struct: sizeof(T_ADETHERMAL_CONFIG) */
    char     i2c_device[260]; /* i2c device name: set to "/dev/i2c/0" for Ti-ARM, set to "/dev/i2c-1" for other */
    char     spi_device[260]; /* spi device name: default "/dev/spidev0.0" */
    char     uart_device[260];/* uart device name: default "/dev/serail0" */
    uint32_t spi_speed;       /* spi rx speed: 2200000 ~ 40000000 (2.2MHz ~ 40MHz), default 4000000 (4MHz) */
    uint8_t  spi_rx_mode;     /* spi rx mode: default 3 */
    uint8_t  spi_rx_bits;     /* spi rx bits: 8 or 16. set 16 for Ti-ARM, set 8 for other */
    uint8_t  spi_word_swap;   /* swap byte-order on word, 0 disable, 1 enable (ex: 0x1234 swap to 0x3412). must set to 1 */
    uint8_t  spi_byte_swap;   /* swap bit-order on byte, 0 disable, 1 enable (ex: 0x23 swap to 0xC4). set 1 for Ti-ARM, set 0 for other */
    uint8_t  thread_priority; /* spi-thread priority: 0 ~ 15, default 0, 15 highest */
    uint8_t  mirror_leftright;/* left-right mirror, 0 disable, 1 enable */
    uint8_t  mirror_downup;   /* down-up mirror, 0 disable, 1 enable */
    uint8_t  optional;        /* bit[0]: germanium glass, 1 has, 0 not
                               * bit[1]: 1 correct mode, 0 normal mode
                               * bit[2]: 1 ignore crc error, 0 discard frame if crc error
                               * bit[3]: 1 read spi outside the SDK, 0 read spi inside the SDK
                               * bit[4~7]: reserve, set to 0 */
} T_ADETHERMAL_CONFIG;

/* raw-data frame information */
typedef struct _T_ADETHERMAL_RAWDATA_FRAME
{
    void*     data_ptr;  /* buffer pointer. the buffer alloced by SDK, NOT to free on user-application */
    uint32_t  data_size; /* bytes */
    uint32_t  width;     /* image width */
    uint32_t  height;    /* image height */
    uint32_t  frame_seq; /* frame seq */
    uint32_t  frame_pts; /* frame time-stamp, milli-seconds */
} T_ADETHERMAL_RAWDATA_FRAME;

/* point raw-data value and corrdiante */
typedef struct _T_ADETHERMAL_POINT
{
    int16_t valid;   /* the point is validate ? */
    int16_t rawdata; /* 16bits raw-data value. call adethermal_rawdata_to_celsius(rawdata) convert to celsius */
    int32_t x;
    int32_t y;
} T_ADETHERMAL_POINT;

/* log callback */
typedef void (*adethermal_log_callback)(uint8_t log_level, const char* log_string);

/* event callback */
typedef int (*adethermal_event_callback)(const char* event, int result, const char* reserve);

/*
** get SDK version
** high 16 bits: major version, low 16 bits: minor version
*/
uint32_t adethermal_sdk_get_version(void);

/*
** SDK initialize
** initializes the global resources, called once when the application is started
**   log_level : show log to 'stdout', 0~3: 0-disalbe, 1-error log only, 2-normal, 3-detail
**   log_proc  : log callback function. if NULL, sdk use 'printf' write to stdout
**   event_proc: event callback function. if NULL, sdk not callback event to application
*/
E_ADETHERMAL_RESULT adethermal_sdk_init(uint8_t log_level, adethermal_log_callback log_proc, adethermal_event_callback event_proc);

/*
** SDK un-initialize
** free global resources, called when the application exit
*/
void adethermal_sdk_uninit(void);

/*
** Open device and start capture raw-data
*/
E_ADETHERMAL_RESULT adethermal_device_open(T_ADETHERMAL_CONFIG* config);

/*
** Stop capture raw-data and close device
*/
void adethermal_device_close(void);

/*
** Run lepton-command
*/
E_ADETHERMAL_RESULT adethermal_device_command(const char* szCommand);
E_ADETHERMAL_RESULT adethermal_device_command2(const char* szCommand, char* szResultValue, void* pInBuffer, uint32_t dwInSize, void* pOutBuffer, uint32_t dwOutBufferSize, uint32_t* pdwActualOutSize);

/*
** Get lepton-parameters
*/
int32_t adethermal_device_get_parameters(const char* type, void* out_buffer, uint32_t out_size);

/*
** input spi-frame to SDK
*/
E_ADETHERMAL_RESULT adethermal_input_spi_frame(void* frame_data, uint32_t width, uint32_t height, uint32_t line_bytes);

/*
** input log-text to SDK, SDK output it to stdout-console, and send to network-client
*/
void adethermal_input_log(uint8_t log_level, const char* log_string);

/*
** get one frame raw-data from device
** if succeed, buffer has beed locked, user must call adethermal_device_unlockbuffer() to unlock buffer
*/
E_ADETHERMAL_RESULT adethermal_device_lockbuffer(T_ADETHERMAL_RAWDATA_FRAME* rawdata);

/*
** unlock raw-data frame buffer
*/
void adethermal_device_unlockbuffer(void);

/*
** raw-data convert to RGB24 image
**   rawdata          : raw-data frame, get from adethermal_device_lockbuffer()
**   palette          : RGB24 palette, 0~2: 0-rainbow, 1-grayscale, 2-ironblack
**   rgb24_buffer     : RGB24 buffer pointer
**   rgb24_buffer_size: RGB24 buffer size, bytes, must >= (width*height*3)
*/
E_ADETHERMAL_RESULT adethermal_rawdata_to_rgb24(const T_ADETHERMAL_RAWDATA_FRAME* rawdata, uint8_t palette, void* rgb24_buffer, uint32_t rgb24_buffer_size);

/*
** 16 bit raw-data value convert to temperature value (celsius)
** raw-data unit: 0.1 Celsius. value 285 is equal to 28.5 Celsius
*/
double adethermal_rawdata_to_celsius(int16_t rawdata_value);
#define RAWDATA_TO_CELSIUS(raw16)  (raw16/10.0)
#define RAWDATA_TO_KELVIN(raw16)   ((raw16/10.0)+273.15)
#define CELSIUS_TO_KELVIN(celsius) (celsius+273.15)
#define KELVIN_TO_CELSIUS(kelvin)  (kelvin-273.15)

/*
** raw-data compress
**   rawdata             : raw-data frame, get from adethermal_device_lockbuffer()
**   compress_buffer     : compress buffer, save the compressed data
**   compress_buffer_size: compress buffer size, bytes, must > (width*height*2)
**   return              : compressed data size, bytes. return 0 if fail
*/
uint32_t adethermal_rawdata_compress(const T_ADETHERMAL_RAWDATA_FRAME* rawdata, void* compress_buffer, uint32_t compress_buffer_size);

/*
** uncompress to raw-data
**   compressed_data: compressed data buffer
**   data_size      : compressed data size, bytes, on 'compressed_data' buffer
**   out_buffer     : buffer pointer. save uncompressed data to this buffer
**   out_buffer_size: output buffer size, bytes.
**   width          : get image width
**   height         : get image height
**   return         : validate raw-data size on 'out_buffer', bytes. return 0 if fail
*/
uint32_t adethermal_rawdata_uncompress(const void* compressed_data, uint32_t data_size, void* out_buffer, uint32_t out_buffer_size, uint32_t* width, uint32_t* height);

/*
** get min/max/avg value on raw-data, and min/max pointe's coordinate
*/
E_ADETHERMAL_RESULT adethermal_rawdata_get_range(const T_ADETHERMAL_RAWDATA_FRAME* rawdata, T_ADETHERMAL_POINT* get_min, T_ADETHERMAL_POINT* get_max, int16_t* get_avg);

#endif // ADETHERMAL_H
