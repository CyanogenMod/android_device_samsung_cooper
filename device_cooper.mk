# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This file is the build configuration for a full Android
# build for sapphire hardware. This cleanly combines a set of
# device-specific aspects (drivers) with a device-agnostic
# product configuration (apps).
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

$(call inherit-product, device/common/gps/gps_eu_supl.mk)

DEVICE_PACKAGE_OVERLAYS := device/samsung/cooper/overlay

# Discard inherited values and use our own instead.
PRODUCT_NAME := cooper
PRODUCT_DEVICE := cooper
PRODUCT_MODEL := GT-S5830

PRODUCT_PACKAGES += \
    libOmxCore \
    libOmxVidEnc \
    FM \
    Torch \
    make_ext4fs \
    FM \
    rzscontrol \
    SamsungServiceMode \
    bdaddr_read \
    toggleshutter \
    dexpreopt \
    e2fsck \
    brcm_patchram_plus \
    copybit.cooper \
    lights.cooper \
    gralloc.cooper \
    setup_fs \
    screencap

ifeq ($(TARGET_PREBUILT_KERNEL),)
	LOCAL_KERNEL := device/samsung/cooper/kernel
else
	LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_PACKAGES += \
       sec_touchscreen.kcm \
       sec_jack.kcm \
       sec_key.kcm

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

# proprietary side of the device
$(call inherit-product-if-exists, vendor/samsung/cooper/cooper-vendor.mk)

# fstab
PRODUCT_COPY_FILES += \
    device/samsung/cooper/prebuilt/vold.fstab:system/etc/vold.fstab

# Init
PRODUCT_COPY_FILES += \
    device/samsung/cooper/init.gt-s5830.rc:root/init.gt-s5830.rc \
    device/samsung/cooper/ueventd.gt-s5830.rc:root/ueventd.gt-s5830.rc

# 3d
PRODUCT_COPY_FILES += \
    device/samsung/cooper/egl.cfg:system/lib/egl/egl.cfg 

# Audio
PRODUCT_COPY_FILES += \
    device/samsung/cooper/prebuilt/AudioFilter.csv:system/etc/AudioFilter.csv \
    device/samsung/cooper/prebuilt/AutoVolumeControl.txt:system/etc/AutoVolumeControl.txt

# WLAN + BT
PRODUCT_COPY_FILES += \
    device/samsung/cooper/prebuilt/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    device/samsung/cooper/prebuilt/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
    device/samsung/cooper/prebuilt/hostapd:system/bin/hostapd \
    device/samsung/cooper/prebuilt/hostapd.conf:system/etc/wifi/hostapd.conf
    


# Install the features available on this device.
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml

#Kernel Modules
PRODUCT_COPY_FILES += \
    device/samsung/cooper/prebuilt/modules/fsr.ko:root/lib/modules/fsr.ko \
    device/samsung/cooper/prebuilt/modules/fsr_stl.ko:root/lib/modules/fsr_stl.ko \
    device/samsung/cooper/prebuilt/modules/sec_param.ko:root/lib/modules/sec_param.ko \
    device/samsung/cooper/prebuilt/get_macaddrs:system/bin/get_macaddrs \
    device/samsung/cooper/prebuilt/cifs.ko:system/lib/modules/cifs.ko \
    device/samsung/cooper/prebuilt/tun.ko:system/lib/modules/tun.ko     
    
#Kernel Modules for Recovery (RFS)
PRODUCT_COPY_FILES += \
    device/samsung/cooper/prebuilt/modules/recovery/fsr.ko:recovery/root/lib/modules/fsr.ko \
    device/samsung/cooper/prebuilt/modules/recovery/fsr_stl.ko:recovery/root/lib/modules/fsr_stl.ko \
    device/samsung/cooper/prebuilt/modules/recovery/rfs_fat.ko:recovery/root/lib/modules/rfs_fat.ko \
    device/samsung/cooper/prebuilt/modules/recovery/rfs_glue.ko:recovery/root/lib/modules/rfs_glue.ko \
    device/samsung/cooper/prebuilt/modules/recovery/sec_param.ko:recovery/root/lib/modules/sec_param.ko
    
#Media profile
PRODUCT_COPY_FILES += \
    device/samsung/cooper/prebuilt/media_profiles.xml:system/etc/media_profiles.xml \
    device/samsung/cooper/prebuilt/audio.conf:system/etc/bluetooth/audio.conf
## keymap
PRODUCT_COPY_FILES += \
    device/samsung/cooper/qwerty.kl:system/usr/keylayout/qwerty.kl \
    device/samsung/cooper/sec_jack.kl:system/usr/keylayout/sec_jack.kl \
    device/samsung/cooper/sec_key.kl:system/usr/keylayout/sec_key.kl \
    device/samsung/cooper/sec_touchscreen.kl:system/usr/keylayout/sec_touchscreen.kl

PRODUCT_PROPERTY_OVERRIDES := \
    keyguard.no_require_sim=true \
    ro.com.android.dateformat=dd-MM-yyyy \
    ro.ril.hsxpa=1 \
    ro.ril.gprsclass=10 \
    ro.media.dec.jpeg.memcap=10000000

PRODUCT_PROPERTY_OVERRIDES += \
    mobiledata.interfaces=pdp0,wlan0,gprs,ppp0 \
    ro.telephony.ril_class=samsung \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=15 \
    ro.com.android.dataroaming=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=160 


# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# This should not be needed but on-screen keyboard uses the wrong density without it.
PRODUCT_PROPERTY_OVERRIDES += \
    qemu.sf.lcd_density=160

PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.google.locationfeatures=1 \
    ro.setupwizard.enable_bypass=1 \
    ro.media.dec.jpeg.memcap=20000000 \
    dalvik.vm.lockprof.threshold=500 \
    dalvik.vm.dexopt-flags=m=y \
    dalvik.vm.heapsize=32m \
    dalvik.vm.execution-mode=int:jit \
    dalvik.vm.dexopt-data-only=1 \
    ro.opengles.version=131072  \
    ro.compcache.default=0

# See comment at the top of this file. This is where the other
# half of the device-specific product definition file takes care
# of the aspects that require proprietary drivers that aren't
# commonly available
$(call inherit-product-if-exists, vendor/samsung/cooper/cooper-vendor-blobs.mk)
