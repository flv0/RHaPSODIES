#include <VistaMath/VistaBoundingBox.h>

#include <HandRenderer.hpp>

#include "HandRenderDraw.hpp"

namespace rhapsodies {
	HandRenderDraw::HandRenderDraw(HandModel *pModelLeft,
								   HandModel *pModelRight,
								   HandModelRep *pModelRep,
								   ShaderRegistry *pReg) :
		m_pModelLeft(pModelLeft),
		m_pModelRight(pModelRight),
		m_pModelRep(pModelRep),
		m_pRenderer(new HandRenderer(pReg)) {

	}

	HandRenderDraw::~HandRenderDraw() {
		delete m_pRenderer;
	}

	bool HandRenderDraw::Do() {
		m_pRenderer->DrawHand(m_pModelLeft,  m_pModelRep);
		m_pRenderer->DrawHand(m_pModelRight, m_pModelRep);
		m_pRenderer->PerformDraw(0, NULL);
		return true;
	}

	bool HandRenderDraw::GetBoundingBox(VistaBoundingBox &bb) {
		bb = VistaBoundingBox(
			VistaVector3D(-10, -10, -10),
			VistaVector3D( 10,  10,  10));
		return true;
	}
}
