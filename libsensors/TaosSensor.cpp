/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include "taos_common.h"

#include <cutils/log.h>

#include "TaosSensor.h"

/*****************************************************************************/

TaosSensor::TaosSensor()
    : SensorBase(TAOS_DEVICE_NAME, "alsprox"),
      mEnabled(0),
      mInputReader(4),
      mPendingMask(0)
{

    memset(mPendingEvents, 0, sizeof(mPendingEvents));

    mPendingEvents[Proximity].version = sizeof(sensors_event_t);
    mPendingEvents[Proximity].sensor = ID_P;
    mPendingEvents[Proximity].type = SENSOR_TYPE_PROXIMITY;

    mPendingEvents[Light].version = sizeof(sensors_event_t);
    mPendingEvents[Light].sensor = ID_L;
    mPendingEvents[Light].type = SENSOR_TYPE_LIGHT;

    open_device();
    if ( (!ioctl(dev_fd, TAOS_IOCTL_ALS_ON)) && (!ioctl(dev_fd, TAOS_IOCTL_PROX_ON))) {
        mEnabled = 1;
        setInitialState();
    }
    if (!mEnabled) {
        close_device();
    }

}

TaosSensor::~TaosSensor() {
    if (mEnabled) {
        enable(Proximity, 0);
        enable(Light, 0);
    }
}

int TaosSensor::setInitialState() {
    struct input_absinfo absinfo;
    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_PROXIMITY), &absinfo)) {
        mPendingEvents[Proximity].distance = indexToValue(absinfo.value);
    }
    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_LIGHT), &absinfo)) {
        mPendingEvents[Light].light = absinfo.value;
    }
    return 0;
}

int TaosSensor::enable(int32_t handle, int en) {
    int what = -1;
        switch (handle) {
            case ID_P: what = Proximity; break;
            case ID_L: what = Light;   break;
        }

    if (uint32_t(what) >= numSensors)
        return -EINVAL;

    int newState = en ? 1 : 0;
    int err = 0;

    if ((uint32_t(newState)<<what) != (mEnabled & (1<<what))) {
        if (!mEnabled) {
                open_device();
        }
        int cmd;

	if (newState  && !(mEnabled & (1<<what)) && !(mEnabled & (1<<Proximity))) {
	     switch (what) {
	           case Proximity: 
		     LOGD_IF(DEBUG,"PROX ON");
		     cmd = TAOS_IOCTL_PROX_ON; 
		     mPreviousDistance = -1;
		     break;
	           case Light:     
		     LOGD_IF(DEBUG,"ALS ON");
		     cmd = TAOS_IOCTL_ALS_ON;  
		     mPreviousLight = -1;
		     break;
	      }
	} else {
	     switch (what) {
	           case Proximity: 
		     cmd = TAOS_IOCTL_PROX_OFF; 
		     LOGD_IF(DEBUG,"PROX OFF"); 
		     break;
                   case Light:     
		     cmd = TAOS_IOCTL_ALS_OFF; 
		     LOGD_IF(DEBUG,"ALS OFF");  
		     break;
	     }
	}
      	err = ioctl(dev_fd, cmd);
	err = err<0 ? -errno : 0;
	LOGE_IF(err, "TAOS_IOCTL_XXX failed (%s)", strerror(-err));
        if (!err) {
            if (en) {
                setInitialState();
		mEnabled |= 1<<what;
            }
	    else {
	       mEnabled &= ~(1<<what);
	       // Turning on the prox sensor disables the light sensor. 
	       // Reinitialise light sensor when prox sensor is disabled if light sensor should still be on.
	       if(mEnabled & (1<<Light)) {
		 LOGD_IF(DEBUG,"Re-enable ALS");
		 ioctl(dev_fd, TAOS_IOCTL_ALS_ON);
	       }
	    }	     
        }
        if (!mEnabled) {
	  LOGD_IF(DEBUG,"closing device");
            close_device();
        }
    }
    return err;
}

bool TaosSensor::hasPendingEvents() const {
     return mPendingMask;
}

int TaosSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;
    int numEventReceived = 0;
    input_event const* event;
    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
	  switch(event->code){

	     case EVENT_TYPE_PROXIMITY:
	       LOGD_IF(DEBUG,"Prox value=%i",event->value);
                   mPendingEvents[Proximity].distance = indexToValue(event->value);
   		   break;
	     case EVENT_TYPE_LIGHT:
	       LOGD_IF(DEBUG,"Light value=%i",event->value);
                   mPendingEvents[Light].light = event->value;
		   break;
	  }

        } else if (type == EV_SYN) {
           int64_t time = timevalToNano(event->time);
	   if (mEnabled & (1<<Proximity) && mPendingEvents[Proximity].distance != mPreviousDistance){
	     *data++ = mPendingEvents[Proximity];
	     mPendingEvents[Proximity].timestamp=time;
	     count--;
	     numEventReceived++;
	     mPreviousDistance = mPendingEvents[Proximity].distance;
	   }
	   if (mEnabled & (1<<Light) && mPendingEvents[Light].light != mPreviousLight){
	     *data++ = mPendingEvents[Light];
	     mPendingEvents[Light].timestamp=time;
	     count--;
	     numEventReceived++;
	     mPreviousLight = mPendingEvents[Light].light;
	   }
        } else {
            LOGE("TaosSensor: unknown event (type=%d, code=%d)",type, event->code);
        }
        mInputReader.next();
    }
    return numEventReceived;
}

float TaosSensor::indexToValue(size_t index) const
{
  return index;
}
