/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009-2011 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
* @file
*   @brief Shared memory interface for reading images.
*
*   This interface is a wrapper around PxSHM.
*
*   @author Lionel Heng  <hengli@inf.ethz.ch>
*
*/

#include "PxSHMImageClient.h"

PxSHMImageClient::PxSHMImageClient()
{
	
}

bool
PxSHMImageClient::init(bool subscribeLatest,
					   PxSHM::Camera cam1, PxSHM::Camera cam2)
{
	this->subscribeLatest = subscribeLatest;
	
	data.reserve(1024 * 1024);

	if (!shm.init(cam1 | cam2, PxSHM::CLIENT_TYPE, 128, 1, 1024 * 1024, 10))
	{
		return false;
	}

	return true;
}

uint64_t
PxSHMImageClient::getTimestamp(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return 0;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.timestamp;
	}
}

uint64_t
PxSHMImageClient::getValidUntil(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return 0;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.valid_until;
	}
}

uint64_t
PxSHMImageClient::getCameraID(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return 0;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.cam_id;
	}
}

uint32_t
PxSHMImageClient::getCameraNo(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return -1;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.cam_no;
	}
}

bool
PxSHMImageClient::getRollPitch(const mavlink_message_t* msg, float& roll, float& pitch)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		roll = img.roll;
		pitch = img.pitch;

		return true;
	}
}

bool
PxSHMImageClient::getRollPitchYaw(const mavlink_message_t* msg, float& roll, float& pitch, float& yaw)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		roll = img.roll;
		pitch = img.pitch;
		yaw = img.yaw;

		return true;
	}
}

bool
PxSHMImageClient::getLocalHeight(const mavlink_message_t* msg, float& height)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		height = img.local_z;

		return true;
	}
}

bool
PxSHMImageClient::getGPS(const mavlink_message_t* msg, float& lat, float& lon, float& alt)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		lat = img.lat;
		lon = img.lon;
		alt = img.alt;

		return true;
	}
}

bool
PxSHMImageClient::readMonoImage(const mavlink_message_t* msg, cv::Mat& img)
{
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	
	if (!shm.bytesWaiting())
	{
		return false;
	}
	
	do
	{
		PxSHM::CameraType cameraType;
		if (!readCameraType(cameraType))
		{
			return false;
		}

		if (cameraType != PxSHM::CAMERA_MONO_8 && cameraType != PxSHM::CAMERA_MONO_24)
		{
			return false;
		}

		if (!readImage(img))
		{
			return false;
		}
	}
	while (shm.bytesWaiting() && subscribeLatest);
	
	return true;
}

bool
PxSHMImageClient::readStereoImage(const mavlink_message_t* msg, cv::Mat& imgLeft, cv::Mat& imgRight)
{
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	
	if (!shm.bytesWaiting())
	{
		return false;
	}
	
	do
	{
		PxSHM::CameraType cameraType;
		if (!readCameraType(cameraType))
		{
			return false;
		}

		if (cameraType != PxSHM::CAMERA_STEREO_8 && cameraType != PxSHM::CAMERA_STEREO_24)
		{
			return false;
		}

		if (!readImage(imgLeft, imgRight))
		{
			return false;
		}
	}
	while (shm.bytesWaiting() && subscribeLatest);
	
	return true;
}

bool
PxSHMImageClient::readKinectImage(const mavlink_message_t* msg, cv::Mat& imgBayer, cv::Mat& imgDepth)
{
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}

	if (!shm.bytesWaiting())
	{
		return false;
	}

	do
	{
		PxSHM::CameraType cameraType;
		if (!readCameraType(cameraType))
		{
			return false;
		}

		if (cameraType != PxSHM::CAMERA_KINECT)
		{
			return false;
		}

		if (!readImage(imgBayer, imgDepth))
		{
			return false;
		}
	}
	while (shm.bytesWaiting() && subscribeLatest);
	
	return true;
}

bool
PxSHMImageClient::readCameraType(PxSHM::CameraType& cameraType)
{
	uint32_t dataLength = shm.readDataPacket(data, 4);
	if (dataLength < 4)
	{
		return false;
	}

	memcpy(&cameraType, &(data[0]), 4);

	return true;
}

bool
PxSHMImageClient::readImage(cv::Mat& img)
{
	uint32_t dataLength = shm.readDataPacket(data);
	if (dataLength <= 20)
	{
		return false;
	}

	int rows, cols, type;
	uint32_t step;

//	memcpy(&cameraType, &(data[0]), 4);
	memcpy(&cols, &(data[4]), 4);
	memcpy(&rows, &(data[8]), 4);
	memcpy(&step, &(data[12]), 4);
	memcpy(&type, &(data[16]), 4);

	if (dataLength != 20 + rows * step)
	{
		// data length is not consistent with image type
		return false;
	}

	cv::Mat temp(rows, cols, type, &(data[20]), step);
	temp.copyTo(img);

	return true;
}

bool
PxSHMImageClient::readImage(cv::Mat& img, cv::Mat& img2)
{
	uint32_t dataLength = shm.readDataPacket(data);
	if (dataLength <= 28)
	{
		return false;
	}

	int rows, cols, type, type2;
	uint32_t step, step2;

//	memcpy(&cameraType, &(data[0]), 4);
	memcpy(&cols, &(data[4]), 4);
	memcpy(&rows, &(data[8]), 4);
	memcpy(&step, &(data[12]), 4);
	memcpy(&type, &(data[16]), 4);
	memcpy(&step2, &(data[20]), 4);
	memcpy(&type2, &(data[24]), 4);

	if (dataLength != 28 + rows * step + rows * step2)
	{
		// data length is not consistent with image type
		return false;
	}

	cv::Mat temp(rows, cols, type, &(data[28]), step);
	temp.copyTo(img);

	cv::Mat temp2(rows, cols, type2, &(data[28 + rows * step]), step2);
	temp2.copyTo(img2);

	return true;
}