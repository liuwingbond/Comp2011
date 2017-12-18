/*
 *       File: pa3.cpp
 * Created on: Nov 1, 2017
 *     Author: Yao Yao (yyaoag@cse.ust.hk)
 *       Note: Implement all functions without changing any function prototype.
 *             This is the only file you should modify and submit.
 */

#include <iostream>
#include "structure.h"
using namespace std;

/*
 * Description: get the pointer to the frame through frame index.
 *
 *       video: input video structure.
 * frame_index: the index of the frame to be extracted.
 *      return: the pointer to the frame or nullptr if the frame does not exsist.
 */
Frame * GetFrame(const Video & video, const int frame_index)
{
	// your implementation
	if (frame_index >= video.num_frames || frame_index < 0)
		return nullptr;
	Frame* currFrame = video.first_frame;
	for (int i = 0; i < frame_index && currFrame->next_frame; i++, currFrame = currFrame->next_frame);
	return currFrame;
}

/*
 * Description: get the pointer to the vehicle through vehicle index.
 *
 *         video: input video structure.
 * vehicle_index: the index of the vehicle to be extracted.
 *        return: the pointer to the vehicle or nullptr if the vehicle does not exsist.
 */
Vehicle * GetVehicle(const Video & video, const int vehicle_index)
{
	// your implementation
	if (vehicle_index >= video.num_vehicles || vehicle_index < 0)
		return nullptr;
	return video.vehicles[vehicle_index];
}

/*
 * Description: get the pointer to vf_info of a vehicle in one frame.
 *
 *     vehicle: input vehicle structure.
 * frame_index: the frame index of the vf_info to be extracted from this vehicle.
 *      return: the pointer to the vf_info or nullptr if the vehicle does not visible in
 *              this frame.
 */
VehicleFrameInfo * GetVFInfo(const Vehicle * vehicle, const int frame_index)
{
	// your implementation
	if (vehicle) {
		VehicleFrameInfo* currVFInfo = vehicle->first_frame_info;
		for (int i = 0; i <= frame_index && i < vehicle->num_visible_frames && currVFInfo; i++, currVFInfo = currVFInfo->next_frame_info) {
			if (currVFInfo->frame_index == frame_index)
				return currVFInfo;
		}
	}
	return nullptr;
}

/*
 * Description: initialize a new frame in the video during frame processing. You need to
 *              create a new frame, and initialize this new frame and add it to the link
 *              list. In this function, you don't need to use hard copy to initialize
 *              Frame.image. Also, you don't need to analyze the vehicles in this frame
 *              (set num_vehicles of this new frame as 0 and all pointers to vehicles as
 *              nullptr).
 *
 *  video: input video structure.
 * return: true if finish and false if all frames has been processed.
 */
bool InitializeNewFrame(Video & video)
{
	// your implementation
	if (video.num_frames == video.num_processed_frames)
		return false;

	Frame* newFrame = new Frame;
	newFrame->num_vehicles = 0;
	newFrame->index = video.num_processed_frames;
	newFrame->next_frame = nullptr;
	for (int i = 0; i < MAX_VEHICLE_NUM; i++)
		newFrame->vehicles[i] = nullptr;

	//newFrame->image = new char*[ROWS];
	newFrame->image = video.raw_data[newFrame->index];
	/*for (int i = 0; i < ROWS; i++) {
		//newFrame->image[i] = new char[COLS];
		for (int j = 0; j < COLS; j++) {
			//newFrame->image[i][j] = video.raw_data[newFrame->index][i][j];
		}
	}*/

	Frame* prevFrame = video.first_frame;
	for (int i = 0; i < video.num_processed_frames && i < video.num_frames && prevFrame->next_frame; i++, prevFrame = prevFrame->next_frame);

	if (video.num_processed_frames == 0)
		video.first_frame = newFrame;
	else
		prevFrame->next_frame = newFrame;

	(video.num_processed_frames)++;
	return true;
}

/*
 * Description: add one vf_info to the video. You need to check if the vf_info, the
 *              corresponding video and the corresponding the vehicle has been in the
 *              video before you add the VFInfo.
 *
 *              video: input video structure.
 * vehicle_frame_info: pointer to the new info to be added. (vf_info)
 *             return: return true if finish and false if
 *                     1. the frame or the vehicle of this vf_info is not in this video or
 *                     2. vf_info has already been in the list of the vehicle.
 */
bool AddVFInfo(Video & video, VehicleFrameInfo * vehicle_frame_info)
{
	// your implementation
	if (vehicle_frame_info->frame_index >= video.num_frames || vehicle_frame_info->vehicle_index >= video.num_vehicles)
		return false;
	if (video.vehicles[vehicle_frame_info->vehicle_index]->first_frame_info == nullptr) {
		video.vehicles[vehicle_frame_info->vehicle_index]->first_frame_info = vehicle_frame_info;
	}
	else {
		VehicleFrameInfo* prevVFI = video.vehicles[vehicle_frame_info->vehicle_index]->first_frame_info;
		for (VehicleFrameInfo* currVFI = prevVFI; currVFI; currVFI = currVFI->next_frame_info) {
			if (currVFI == vehicle_frame_info || prevVFI->frame_index == vehicle_frame_info->frame_index && prevVFI->vehicle_index == vehicle_frame_info->vehicle_index)
				return false;
			prevVFI = currVFI;
		}
		prevVFI->next_frame_info = vehicle_frame_info;
	}
	vehicle_frame_info->next_frame_info = nullptr;

	(GetVehicle(video, vehicle_frame_info->vehicle_index)->num_visible_frames)++;

	Frame* currFrame = GetFrame(video, vehicle_frame_info->frame_index);
	currFrame->vehicles[vehicle_frame_info->vehicle_index] = video.vehicles[vehicle_frame_info->vehicle_index];
	(currFrame->num_vehicles)++;
	return true;
}

/*
 * Description: track the vehicle in the new frame and return the new info. The speed
 *              is the distance between positions of the vehicle in two frames and note
 *              that 1. the maximum speed of one vehicle is MAX_SPEED and 2. the minimum
 *              distance between vehicles in the same lane is MAX_SPEED and 3. the vehicle
 *              is assumed always to stay in the same lane.
 *       vehicle: pointer to the vehicle to be tracked.
 * current_frame: pointer to the frame the vehicle to be tracked in.
 *    prev_frame: pointer to the previous frame the vehicle was in.
 *        return: the pointer to the new info or nullptr if 1. the vehicle is not in
 *                prev_frame or 2. the vehicle left scope of current frame.
 */
VehicleFrameInfo * TrackVehicle(const Vehicle * vehicle, const Frame * current_frame, const Frame * prev_frame)
{
	// your implementation
	for (int i = 0; i < COLS; i++) {
		if (i == COLS - 1)
			return nullptr;
		if (prev_frame->vehicles[i] == vehicle)
			break;
	}

	VehicleFrameInfo *prevVFI = GetVFInfo(vehicle, prev_frame->index);

	int currFrame_Position = prevVFI->position[1];

	for (int i = 1; i <= COLS; i++) {
		if (current_frame->image[prevVFI->position[0]][i] == '*' && i >= currFrame_Position) {
			currFrame_Position = i;
			break;
		}
		if (i >= COLS)
			return nullptr;
	}
	
	VehicleFrameInfo *newVFI = new VehicleFrameInfo;
	newVFI->frame_index = current_frame->index;
	newVFI->next_frame_info = nullptr;
	newVFI->position[0] = prevVFI->position[0];
	newVFI->position[1] = currFrame_Position;
	newVFI->speed = currFrame_Position - prevVFI->position[1];
	if (newVFI->speed > MAX_SPEED)
		newVFI->speed = MAX_SPEED;
	newVFI->vehicle_index = vehicle->index;

	return newVFI;
}

/*
 * Description: find and add new vehicles in the last frame of the frame list. You need to 1. find the
 *              new vehicles in the first colume of each lane, and 2. create and initialize a new vehicle
 *              is a new vehicle is found and, 3. add the new vehicle to the video and, 4. create a
 *              vf_info for the new vehicle in this frame and, 5. add the vf_info to the video.
 *
 *  video: input video structure.
 * return: true is finish or false if the frame index is invalid.
 */
bool FindAndAddNewVehicles(Video & video)
{
	// your implementation
	Frame *lastFrame = GetFrame(video, video.num_processed_frames - 1);

	if (lastFrame->index > video.num_frames)
		return false;

	for (int i = 0; i < ROWS; i++) {
		if (lastFrame->image[i][0] == '*') {
			Vehicle *newVehicle = new Vehicle;
			newVehicle->index = video.num_vehicles;
			newVehicle->num_visible_frames = 1;

			lastFrame->vehicles[newVehicle->index] = newVehicle;
			(lastFrame->num_vehicles)++;
			video.vehicles[(video.num_vehicles)++] = newVehicle;

			VehicleFrameInfo *newVFI = new VehicleFrameInfo;
			newVFI->frame_index = lastFrame->index;
			newVFI->next_frame_info = nullptr;
			newVFI->position[0] = i;
			newVFI->position[1] = 0;
			newVFI->speed = 1;
			newVFI->vehicle_index = newVehicle->index;

			newVehicle->first_frame_info = newVFI;
		}
	}
	return true;
	// Hints:
	// detect new vehicles lane by lane

		// check if there is a new vehicle in the lane

		// construct and add a new vehicle

		// construct and add a new vf_info
}

/*
 * Description: calcute the average speed of all vehicles in all their visible frames (the average of all speeds).
 *
 *  video: input video structure.
 * return: the average speed or 0 if no vehicle is in the video.
 */
double AverageRoadSpeed(Video & video)
{
	// your implementation
	if (video.num_vehicles == 0)
		return 0;

	double AverageRoadSpeed = 0;
	int VFICounter = 0;
	for (int i = 0; i < video.num_vehicles; i++) {
		for (VehicleFrameInfo* currVFI = video.vehicles[i]->first_frame_info; currVFI; currVFI = currVFI->next_frame_info, VFICounter++)
			AverageRoadSpeed += currVFI->speed;
	}
	return AverageRoadSpeed / VFICounter;
}

/*
 * Description: clean all memories of the video, including raw_data, all frames, vehicles and vf_infos.
 *
 *  video: input video to be clean.
 * return: no return.
 */
void CleanVideo(Video & video)
{
	// your implementation
	for (int i = 0; i < video.num_vehicles; i++) {
		for (VehicleFrameInfo* currVFI = video.vehicles[i]->first_frame_info, *prevVFI = currVFI->next_frame_info; currVFI; prevVFI = prevVFI->next_frame_info) {
			delete currVFI;
			currVFI = prevVFI;
			if (!currVFI)
				break;
		}
		delete video.vehicles[i];
	}

	for (Frame* currFrame = video.first_frame, *prevFrame = currFrame->next_frame; currFrame; prevFrame = prevFrame->next_frame) {
		delete currFrame;
		currFrame = prevFrame;
		if (!currFrame)
			break;
	}

	for (int i = 0; i < video.num_frames; i++) {
		for (int j = 0; j < ROWS; j++) {
			delete [] video.raw_data[i][j];
		}
		delete [] video.raw_data[i];
	}
	delete [] video.raw_data;
}
