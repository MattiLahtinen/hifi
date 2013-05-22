//
//  Transmitter.h
//  hifi
//
//  Created by Philip Rosedale on 5/20/13.
//
//

#ifndef __hifi__Transmitter__
#define __hifi__Transmitter__

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstring>
#include "world.h"

class Transmitter
{
public:
    Transmitter();
    void render();
    void resetLevels();
    void renderLevels(int width, int height);
    bool isConnected() { return _isConnected; };
    const glm::vec3 getLastRotationRate() const { return _lastRotationRate; };
    const glm::vec3 getLastAcceleration() const { return _lastRotationRate; };
    const glm::vec3 getEstimatedRotation() const { return _estimatedRotation; };
    void processIncomingData(unsigned char* packetData, int numBytes);

private:
    bool _isConnected;
    glm::vec3 _lastRotationRate;
    glm::vec3 _lastAcceleration;
    glm::vec3 _estimatedRotation;

#endif /* defined(__hifi__Transmitter__) */
};