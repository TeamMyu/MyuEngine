#pragma once
#include "GL/Camera2D.h"

class CameraService {
public:
    static void registerMainCamera(Camera2D* camera) {
        mainCamera = camera;
    }

    static Camera2D* getMainCamera() {
        return mainCamera;
    }

private:
    static Camera2D* mainCamera;
};