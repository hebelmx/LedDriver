# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/suyuquan/esp/esp-idf/components/bootloader/subproject"
  "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader"
  "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader-prefix"
  "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader-prefix/tmp"
  "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader-prefix/src/bootloader-stamp"
  "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader-prefix/src"
  "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/ESP-IDF/usb_msc_wireless_disk/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
