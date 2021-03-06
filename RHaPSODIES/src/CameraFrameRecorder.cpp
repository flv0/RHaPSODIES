#include <string>
#include <fstream>

#include <VistaBase/VistaTimer.h>

#include "CameraFrameRecorder.hpp"

namespace rhapsodies {
	CameraFrameRecorder::CameraFrameRecorder() {

	}

	
	void CameraFrameRecorder::StartRecording() {
		std::string sFile = "resources/recordings/";
		sFile += std::to_string(
			VistaTimer::GetStandardTimer().GetSystemTime());
		sFile += ".dump";

		m_oStream.open(sFile, std::ios_base::out | std::ios_base::binary);

		m_tStart = VistaTimer::GetStandardTimer().GetSystemTime();
	}
	
	void CameraFrameRecorder::StopRecording() {
		m_oStream.close();
	}

	void CameraFrameRecorder::RecordFrames(
		  const unsigned char  *colorFrame,
		  const unsigned short *depthFrame,
		  const float          *uvMapFrame) {
		VistaType::systemtime tDelta =
			VistaTimer::GetStandardTimer().GetSystemTime() - m_tStart;

		m_oStream.write((const char*)(&tDelta), 8);
		m_oStream.write((const char*)(colorFrame), 320*240*3);
		m_oStream.write((const char*)(depthFrame), 320*240*2);
		m_oStream.write((const char*)(uvMapFrame), 320*240*4*2);
		m_oStream.flush();
	}
}
