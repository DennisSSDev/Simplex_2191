#include "AppClass.h"
void Application::InitVariables(void)
{

	uint8_t count = 0;
	while (count < 21) 
	{
		m_pFaceArr[count] = new MyMesh();
		m_pFaceArr[count]->GenerateCube(1.0f, C_BLACK);
		++count;
	}
	count = 0;
	while (count < 4) 
	{
		m_pMiddleArr[count] = new MyMesh();
		m_pMiddleArr[count]->GenerateCube(1.0f, C_BLACK);
		++count;
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	static float value = 0.0f;
	matrix4 m4Translate = glm::translate(IDENTITY_M4, vector3(value, 2.0f, 3.0f));
	value += 0.01f;

	//matrix4 m4Model = m4Translate * m4Scale;
	matrix4 m4Model = m4Scale * m4Translate;

	static float translationX = 0.f;
	static float cosineInc = 0.f;
	if (cosineInc > 360.f) {
		cosineInc = 0.f;
	}
	uint8_t count = 0;
	while (count < 21) 
	{
		m_pFaceArr[count]->Render(m4Projection, m4View, glm::translate(IDENTITY_M4, vector3(m_cubePositions[count].x + translationX, m_cubePositions[count].y + cos((cosineInc * PI) / 180.f), 0.f)));
		m_pFaceArr[count]->Render(m4Projection, m4View, glm::translate(IDENTITY_M4, vector3(m_cubePositions[count].x * -1.f + 10.f + translationX, m_cubePositions[count].y + cos((cosineInc * PI) / 180.f), 0.f)));
		count++;
	}

	count = 0; 
	while (count < 4) 
	{
		m_pMiddleArr[count]->Render(m4Projection, m4View, glm::translate(IDENTITY_M4, vector3(m_middlePositions[count].x + translationX, m_middlePositions[count].y + cos((cosineInc * PI) / 180.f), 0.f)));
		count++;
	}
	cosineInc += 1.5f;
	translationX += 0.01f;

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	uint8_t count = 0;
	while (count < 21)
	{
		SafeDelete(m_pFaceArr[count]);
		++count;
	}
	count = 0;
	while (count < 4)
	{
		SafeDelete(m_pMiddleArr[count]);
		++count;
	}

	//release GUI
	ShutdownGUI();
}