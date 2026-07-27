#include <magic_enum/magic_enum.hpp>
#include <iostream>
#include <string>
#include <QCoreApplication>
#include "http_server.hpp"

std::string getReadableVersion(uint64_t integer)
{
    ULARGE_INTEGER version;
    version.QuadPart = integer;
    std::string str;
    str += std::to_string(HIWORD(version.HighPart)) + ".";
    str += std::to_string(LOWORD(version.HighPart)) + ".";
    str += std::to_string(HIWORD(version.LowPart)) + ".";
    str += std::to_string(LOWORD(version.LowPart));

    return str;
}

void init()
{
    ctl_init_args_t CtlInitArgs;
    ctl_api_handle_t hAPIHandle;

    CtlInitArgs.AppVersion = CTL_MAKE_VERSION(CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION);
    CtlInitArgs.flags = CTL_INIT_FLAG_USE_LEVEL_ZERO;
    CtlInitArgs.Size = sizeof(CtlInitArgs);
    CtlInitArgs.Version = 0;
    ZeroMemory(&CtlInitArgs.ApplicationUID, sizeof(ctl_application_id_t));

    ctl_result_t res = ctlInit(&CtlInitArgs, &hAPIHandle);
    if (res != CTL_RESULT_SUCCESS)
        std::cout << "Can't initialize the API" << std::endl;
    else {
        // Enumerate all adapters
        ctl_device_adapter_handle_t *hDevices = nullptr;
        uint32_t Adapter_count = 0;
        ctlEnumerateDevices(hAPIHandle,&Adapter_count, hDevices);
        hDevices = (ctl_device_adapter_handle_t *)malloc(sizeof(ctl_device_adapter_handle_t) * Adapter_count);
        ctlEnumerateDevices(hAPIHandle,&Adapter_count, hDevices);

        //Use the handle to call the api's

        std::cout << "Adapter count: " << Adapter_count << std::endl;

        for(int index = 0; index < Adapter_count; index++) {
            // Get Adapter Properties
            ctl_device_adapter_properties_t StDeviceAdapterProperties = { 0 };

            //OS specific initialization.This is an Windows example

            StDeviceAdapterProperties.Size           = sizeof(ctl_device_adapter_properties_t);
            StDeviceAdapterProperties.pDeviceID      = malloc(sizeof(LUID));
            StDeviceAdapterProperties.device_id_size = sizeof(LUID);
            ctlGetDeviceProperties(hDevices[index], &StDeviceAdapterProperties);
            std::cout << index << ": " << StDeviceAdapterProperties.name;
            std::cout << " (type: " << magic_enum::enum_name(StDeviceAdapterProperties.device_type);
            std::cout << " | version: " << getReadableVersion(StDeviceAdapterProperties.driver_version);
            std::cout << " | fcts: " << StDeviceAdapterProperties.supported_subfunction_flags << ")" << std::endl;

            free(StDeviceAdapterProperties.pDeviceID);

            uint32_t pSensorCount;
            ctl_temp_handle_t *phTemps = nullptr;
            ctlEnumTemperatureSensors(hDevices[index], &pSensorCount, phTemps);
            phTemps = (ctl_temp_handle_t *)malloc(sizeof(ctl_temp_handle_t) * pSensorCount);
            ctlEnumTemperatureSensors(hDevices[index], &pSensorCount, phTemps);

            std::cout << "  Sensor count: " << pSensorCount << std::endl;

            for (int iSens = 0; iSens < pSensorCount; iSens++) {
                double temp;

                ctl_temp_properties_t tempProp;
                tempProp.Size = sizeof(ctl_temp_properties_t);
                tempProp.Version = 0;

                ctl_result_t resTemp = ctlTemperatureGetProperties(phTemps[iSens], &tempProp);

                if (resTemp == CTL_RESULT_SUCCESS) {
                    ctlTemperatureGetState(phTemps[iSens], &temp);
                    std::cout << "  " << magic_enum::enum_name(tempProp.type) << ": " << temp << "°C - Max: " << tempProp.maxTemperature << "°C" << std::endl;
                } else {
                    std::cout << "  Error: " << "0x" << std::hex << resTemp << std::endl;
                }
            }

            free(phTemps);

            uint32_t pFansCount;
            ctl_fan_handle_t *phFans = nullptr;
            ctlEnumFans(hDevices[index], &pFansCount, phFans);
            phFans = (ctl_fan_handle_t *)malloc(sizeof(ctl_fan_handle_t) * pFansCount);
            ctlEnumFans(hDevices[index], &pFansCount, phFans);

            std::cout << "  Fans count: " << pFansCount << std::endl;

            for (int iFan = 0; iFan < pFansCount; iFan++) {
                ctl_fan_config_t fanConfig;
                fanConfig.Size = sizeof(ctl_fan_config_t);
                fanConfig.Version = 0;

                ctl_result_t resFan = ctlFanGetConfig(phFans[iFan], &fanConfig);

                if (resFan == CTL_RESULT_SUCCESS) {
                    int rpm;
                    ctlFanGetState(phFans[iFan], ctl_fan_speed_units_t::CTL_FAN_SPEED_UNITS_RPM, &rpm);

                    std::cout << "  Mode: " << magic_enum::enum_name(fanConfig.mode);
                    std::cout << " " << rpm << " RPMs";
                    std::cout << " (Fixed: " << fanConfig.speedFixed.speed << " " << magic_enum::enum_name(fanConfig.speedFixed.units);
                    std::cout << " | Table: " << fanConfig.speedTable.numPoints << " points)" << std::endl;
                } else {
                    std::cout << "  Error: " << "0x" << std::hex << resFan << std::endl;
                }
            }

            free(phFans);
        }

        free(hDevices);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Set up code that uses the Qt event loop here.
    // Call app.quit() or app.exit() to quit the application.
    // A not very useful example would be including
    // #include <QTimer>
    // near the top of the file and calling
    // QTimer::singleShot(5000, &a, &QCoreApplication::quit);
    // which quits the application after 5 seconds.

    // If you do not need a running Qt event loop, remove the call
    // to app.exec() or use the Non-Qt Plain C++ Application template.

    //init();

    //return app.exec();
    return http_run();
}
