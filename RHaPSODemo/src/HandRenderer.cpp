#include <GL/glew.h>

#include <VistaBase/VistaStreamUtils.h>
#include <VistaBase/VistaMathBasics.h>
#include <VistaBase/VistaVector3D.h>

#include <VistaMath/VistaBoundingBox.h>

#include <VistaKernel/GraphicsManager/VistaGeometryFactory.h>

#include <ShaderRegistry.hpp>

#include <HandModel.hpp>

#include "HandRenderer.hpp"

namespace {
	const std::string sLogPrefix = "[HandRenderer] ";
}

namespace rhapsodies {
	HandRenderer::HandRenderer(HandModel *pModelLeft,
							   HandModel *pModelRight,
							   ShaderRegistry *pReg) :
		m_pModelLeft(pModelLeft),
		m_pModelRight(pModelRight),
		m_pShaderReg(pReg) {

		std::vector<VistaIndexedVertex> vIndices;
		std::vector<VistaVector3D> vCoords;
		std::vector<VistaVector3D> vTexCoords;
		std::vector<VistaVector3D> vNormals;
		std::vector<float> vCoordsFloat;
		std::vector<float> vTexCoordsFloat;
		std::vector<float> vNormalsFloat;
		std::vector<VistaColor> vColors;

		VistaGeometryFactory::CreateEllipsoidData(
			&vIndices, &vCoords, &vTexCoords, &vNormals, &vColors);

		// generate vertex list from indices
		std::vector<VistaIndexedVertex>::iterator it;
		for( it = vIndices.begin() ; it != vIndices.end() ; ++it ) {
			for( int dim = 0 ; dim < 3 ; dim++ ) {
				m_vSphereVertexData.push_back(
					vCoords[it->GetCoordinateIndex()][dim]);
			}
		}

		vIndices.clear();
		vColors.clear();
		VistaGeometryFactory::CreateConeData(
			&vIndices, &vCoordsFloat, &vTexCoordsFloat,
			&vNormalsFloat, &vColors);

		// generate vertex list from indices
		for( it = vIndices.begin() ; it != vIndices.end() ; ++it ) {
			for( size_t dim = 0 ; dim < 3 ; dim++ ) {
				m_vCylinderVertexData.push_back(
					vCoordsFloat[3*it->GetCoordinateIndex()+dim]);
			}
		}

		PrepareVertexBufferObjects();
	}

	void HandRenderer::PrepareVertexBufferObjects() {

		// generate vertex array object names
		glGenVertexArrays(BUFFER_OBJECT_LAST, m_idVertexArrayObjects);
		// generate vertex buffer object names
		glGenBuffers(BUFFER_OBJECT_LAST, m_idBufferObjects);

		// set vertex attrib pointer for sphere and fill with static
		// data
		glBindVertexArray(m_idVertexArrayObjects[SPHERE]);
		glBindBuffer(GL_ARRAY_BUFFER, m_idBufferObjects[SPHERE]);
		glBufferData(GL_ARRAY_BUFFER,
					 sizeof(float)*m_vSphereVertexData.size(),
					 &m_vSphereVertexData[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// set vertex attrib pointer for cylinder and fill with static
		// data
		glBindVertexArray(m_idVertexArrayObjects[CYLINDER]);
		glBindBuffer(GL_ARRAY_BUFFER, m_idBufferObjects[CYLINDER]);
		glBufferData(GL_ARRAY_BUFFER,
					 sizeof(float)*m_vCylinderVertexData.size(),
					 &m_vCylinderVertexData[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		
		// unbind everything for now
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	
	bool HandRenderer::Do() {
		DrawHand(m_pModelLeft);
		DrawHand(m_pModelRight);
		
		return true;
	}

	void HandRenderer::DrawSphere(VistaTransformMatrix matModel,
								  GLint locUniform) {
		glUniformMatrix4fv(locUniform, 1, false, matModel.GetData());
		glDrawArrays(GL_TRIANGLES, 0, m_vSphereVertexData.size()/3);
	}

	void HandRenderer::DrawCylinder(VistaTransformMatrix matModel,
									GLint locUniform) {
		glUniformMatrix4fv(locUniform, 1, false, matModel.GetData());
		glDrawArrays(GL_TRIANGLES, 0, m_vCylinderVertexData.size()/3);
	}

	void HandRenderer::DrawFinger(VistaTransformMatrix matOrigin,
								  float fFingerDiameter, float fLRFactor,
								  float fAng1F, float fAng1A, float fLen1,
								  float fAng2, float fLen2,
								  float fAng3, float fLen3,
								  bool bThumb) {
		
		GLint idProgramS = m_pShaderReg->GetProgram("vpos_green");
		GLint idProgramC = m_pShaderReg->GetProgram("vpos_blue");
		GLint locUniformS = glGetUniformLocation(idProgramS,
												 "model_transform");
		GLint locUniformC = glGetUniformLocation(idProgramC,
												 "model_transform");

		// @todo invert axis here for left/right hand?
		// rotate locally around X for flexion/extension
		VistaAxisAndAngle aaaX = VistaAxisAndAngle(VistaVector3D(1,0,0), 0.0);
		// rotate locally around Z for adduction/abduction
		VistaAxisAndAngle aaaZ = VistaAxisAndAngle(VistaVector3D(0,0,1), 0.0);
		
		VistaTransformMatrix matModel;  // final applied transform
		VistaTransformMatrix matTransform; // auxiliary for
										   // accumulative transforms

		VistaTransformMatrix matSphereScale; // reused sphere scale
		matSphereScale.SetToScaleMatrix(fFingerDiameter);
		
		glUseProgram(idProgramS);
		glBindVertexArray(m_idVertexArrayObjects[SPHERE]);

		// start at first joint
		matModel = matOrigin * matSphereScale;
		if(!bThumb) {
			DrawSphere(matModel, locUniformS);
		}

		// first joint flexion rotation
		aaaX.m_fAngle = Vista::DegToRad(-fAng1F);
		matTransform = VistaTransformMatrix(aaaX);
		matOrigin *= matTransform;

		// first joint abduction rotation
		aaaZ.m_fAngle = Vista::DegToRad(fAng1A*fLRFactor);
		matTransform = VistaTransformMatrix(aaaZ);
		matOrigin *= matTransform;
		
		// move to center of first segment
		matTransform.SetToTranslationMatrix(
			VistaVector3D(0, fLen1/1000.0f/2.0f, 0));
		matOrigin *= matTransform;

		// set scale and draw first segment
		if(bThumb) {
			matTransform.SetToScaleMatrix(
				fFingerDiameter*1.5f,
				fLen1/1000.0f*1.5f,
				fFingerDiameter*1.5f);
			matModel = matOrigin * matTransform;

			DrawSphere(matModel, locUniformC);
		}
		else {
			matTransform.SetToScaleMatrix(
				fFingerDiameter, fLen1/1000.0f, fFingerDiameter);
			matModel = matOrigin * matTransform;

			glUseProgram(idProgramC);
			glBindVertexArray(m_idVertexArrayObjects[CYLINDER]);
			DrawCylinder(matModel, locUniformC);
		}

		// move to second joint
		matTransform.SetToTranslationMatrix(
			VistaVector3D(0, fLen1/1000.0f/2.0f, 0));
		matOrigin *= matTransform;

		// draw scaled sphere
		glUseProgram(idProgramS);
		glBindVertexArray(m_idVertexArrayObjects[SPHERE]);
		matModel = matOrigin * matSphereScale;
		DrawSphere(matModel, locUniformS);

		// second joint flexion rotation
		aaaX.m_fAngle = Vista::DegToRad(-fAng2);
		matTransform = VistaTransformMatrix(aaaX);
		matOrigin *= matTransform;

		// move to center of second segment
		matTransform.SetToTranslationMatrix(
			VistaVector3D(0, fLen2/1000.0f/2.0f, 0)); // rotation missing here
		matOrigin *= matTransform;

		// set scale and draw second segment cylinder
		glUseProgram(idProgramC);
		glBindVertexArray(m_idVertexArrayObjects[CYLINDER]);
		matTransform.SetToScaleMatrix(
			fFingerDiameter, fLen2/1000.0f, fFingerDiameter);
		matModel = matOrigin * matTransform;
		DrawCylinder(matModel, locUniformC);

		// move to third joint
		matTransform.SetToTranslationMatrix(
			VistaVector3D(0, fLen2/1000.0f/2.0f, 0)); // rotation missing here
		matOrigin *= matTransform;

		// draw scaled sphere
		glUseProgram(idProgramS);
		glBindVertexArray(m_idVertexArrayObjects[SPHERE]);
		matModel = matOrigin * matSphereScale;
		DrawSphere(matModel, locUniformS);

		// third joint flexion rotation
		aaaX.m_fAngle = Vista::DegToRad(-fAng3);
		matTransform = VistaTransformMatrix(aaaX);
		matOrigin *= matTransform;
		
		// move to center of third segment
		matTransform.SetToTranslationMatrix(
			VistaVector3D(0, fLen3/1000.0f/2.0f, 0)); // rotation missing here
		matOrigin *= matTransform;

		// set scale and draw third segment cylinder
		glUseProgram(idProgramC);
		glBindVertexArray(m_idVertexArrayObjects[CYLINDER]);
		matTransform.SetToScaleMatrix(
			fFingerDiameter, fLen3/1000.0f, fFingerDiameter);
		matModel = matOrigin * matTransform;
		DrawCylinder(matModel, locUniformC);

		// move to tip
		matTransform.SetToTranslationMatrix(
			VistaVector3D(0, fLen3/1000.0f/2.0f, 0)); // rotation missing here
		matOrigin *= matTransform;

		// draw scaled sphere
		glUseProgram(idProgramS);
		glBindVertexArray(m_idVertexArrayObjects[SPHERE]);
		matModel = matOrigin * matSphereScale;
		DrawSphere(matModel, locUniformS);
	}

	void HandRenderer::DrawHand(HandModel *pModel) {
		// measure timings!

		GLint idProgramS = m_pShaderReg->GetProgram("vpos_green");
		GLint idProgramC = m_pShaderReg->GetProgram("vpos_blue");
		GLint locUniformS = glGetUniformLocation(idProgramS,
												 "model_transform");
		GLint locUniformC = glGetUniformLocation(idProgramC,
												 "model_transform");

		VistaTransformMatrix matModel;
		VistaTransformMatrix matTransform;
		VistaTransformMatrix matOrigin;

		// these go to extra vis parameter class for model pso yo
		float fPalmWidth        = 0.08;
		float fPalmBottomRadius = 0.02;
		float fPalmDiameter     = fPalmWidth/2.0f;
		float fFingerDiameter   = fPalmWidth/4.0f;

		// for now we average the metacarpal lengths for palm height
		float fPalmHeight =
			(pModel->GetExtent(HandModel::I_MC) +
			 pModel->GetExtent(HandModel::M_MC) +
			 pModel->GetExtent(HandModel::R_MC) +
			 pModel->GetExtent(HandModel::L_MC))/4.0f/1000.0f;

		float fLRFactor =
			(pModel->GetType() == HandModel::LEFT_HAND) ?
			-1.0f : 1.0f;
		
		// for now we will upload the same model transform matrix in
		// advance to calling glDrawArrays. If this turns out to be
		// slow, we might upload all the model matrices at once at the
		// beginning and use DrawArraysInstanced to look up the
		// specific matrix in the vertex shader by the instance id.

		glUseProgram(idProgramS);
		glBindVertexArray(m_idVertexArrayObjects[SPHERE]);

		matOrigin.Compose(
			pModel->GetPosition(),
			pModel->GetOrientation(),
			VistaVector3D(1,1,1));
		
		// bottom palm cap
		matTransform.Compose(
			VistaVector3D(0, fPalmBottomRadius, 0),
			VistaQuaternion(),
			VistaVector3D(fPalmWidth,
						  fPalmBottomRadius*2.0f,
						  fPalmDiameter));
		matModel = matOrigin * matTransform;		
		DrawSphere(matModel, locUniformS);

		// top palm cap
		matTransform.Compose(
			VistaVector3D(0, fPalmBottomRadius+fPalmHeight, 0),
			VistaQuaternion(),
			VistaVector3D(fPalmWidth,
						  fPalmBottomRadius*2.0f,
						  fPalmDiameter));
		matModel = matOrigin * matTransform;		
		DrawSphere(matModel, locUniformS);
		
		// palm cylinder
		matTransform.Compose(
			VistaVector3D(0, fPalmBottomRadius + fPalmHeight/2.0f, 0),
			VistaQuaternion(),
			VistaVector3D(fPalmWidth, fPalmHeight, fPalmDiameter));
		matModel = matOrigin * matTransform;		

		glUseProgram(idProgramC);
		glBindVertexArray(m_idVertexArrayObjects[CYLINDER]);
		DrawCylinder(matModel, locUniformC);

		// @todo: if drawing turns out to be a significant bottleneck,
		// we need to adopt the approach of pre-calculating all the
		// primitive transforms, upload them as UBO's and doing
		// instances drawing on the GPU side
		
		// draw the fingers
		for(int i = 0 ; i < 4 ; i++) {
			matTransform.SetToTranslationMatrix(
				VistaVector3D((-fPalmWidth/2.0f + fPalmWidth/8.0f +
							  i*fPalmWidth/4.0f) * fLRFactor,
							  fPalmBottomRadius + fPalmHeight,
							  0));
			
			DrawFinger(
				matOrigin * matTransform,
				fFingerDiameter, fLRFactor,
				pModel->GetJointAngle(4*(1+i)),
				pModel->GetJointAngle(4*(1+i)+1),
				pModel->GetExtent(3+4*i+1),
				pModel->GetJointAngle(4*(1+i)+2),
				pModel->GetExtent(3+4*i+2),
				pModel->GetJointAngle(4*(1+i)+3),
				pModel->GetExtent(3+4*i+3),
				false);
		}

		// draw the thumb
		matTransform.SetToTranslationMatrix(
			VistaVector3D((-fPalmWidth/2.0f + fPalmWidth/8.0f)*fLRFactor,
						  fPalmBottomRadius,
						  0));
			
		DrawFinger(
			matOrigin * matTransform,
			fFingerDiameter*1.2, fLRFactor,
			pModel->GetJointAngle(HandModel::T_CMC_F),
			pModel->GetJointAngle(HandModel::T_CMC_A),
			pModel->GetExtent(HandModel::T_MC),
			pModel->GetJointAngle(HandModel::T_MCP),
			pModel->GetExtent(HandModel::T_PP),
			pModel->GetJointAngle(HandModel::T_IP),
			pModel->GetExtent(HandModel::T_DP),
			true);	

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	
	bool HandRenderer::GetBoundingBox(VistaBoundingBox &bb) {
		bb = VistaBoundingBox(
			VistaVector3D(-10, -10, -10),
			VistaVector3D( 10,  10,  10));
		return true;
	}	
	
	// HandModel* HandRenderer::GetModel() {
	// 	return m_pModel;
	// }

	// void HandRenderer::SetModel(HandModel* pModel) {
	// 	m_pModel = pModel;
	// }
}