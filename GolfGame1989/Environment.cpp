#include "pch.h"
#include "Environment.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>


Environment::Environment()
{
    bool result = InitializeTerrain();
    LoadEnvironmentData();
    CreateDataStrings();
    const int startEnviron = 0;  // ToDo: add error checking 
    m_currentEnviron = m_environs[startEnviron];
    BuildFlagVertex(m_environs[startEnviron].holePosition);
    BuildHoleVertex(m_environs[startEnviron].holePosition);
      
    LoadFixtureBucket();
}

void Environment::BuildFlagVertex(DirectX::SimpleMath::Vector3 aPos)
{
    const float poleHeight = .1;
    const float flagWidth = .02;
    const float flagHeight = .01;
    const DirectX::XMVECTORF32 flagColor = DirectX::Colors::Red;
    const DirectX::XMVECTORF32 poleColor = DirectX::Colors::White;

    DirectX::SimpleMath::Vector3 poleBase = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 poleTop = poleBase;
    poleTop.y += poleHeight;
    DirectX::SimpleMath::Vector3 flagTip = poleTop;
    flagTip.y -= flagHeight;
    flagTip.x -= flagWidth;
    flagTip.z -= flagWidth;
    double windDirection = GetWindDirection();
    flagTip = DirectX::SimpleMath::Vector3::Transform(flagTip, DirectX::SimpleMath::Matrix::CreateRotationY(static_cast<float>(windDirection)));

    DirectX::SimpleMath::Vector3 flagBottom = poleTop;
    flagBottom.y -= flagHeight + flagHeight;

    DirectX::VertexPositionColor poleBaseVertex(poleBase + aPos, poleColor);
    DirectX::VertexPositionColor poleTopVertex(poleTop + aPos, poleColor);
    DirectX::VertexPositionColor flagTopVertex(poleTop + aPos, flagColor);
    DirectX::VertexPositionColor flagTipVertex(flagTip + aPos, flagColor);
    DirectX::VertexPositionColor flagBottomVertex(flagBottom + aPos, flagColor);

    m_flagVertex.clear();
    m_flagVertex.push_back(poleBaseVertex);
    m_flagVertex.push_back(poleTopVertex);
    m_flagVertex.push_back(flagTopVertex);
    m_flagVertex.push_back(flagTipVertex);
    m_flagVertex.push_back(flagBottomVertex);
}

void Environment::BuildHoleVertex(DirectX::SimpleMath::Vector3 aPos)
{
    m_holeVertex.clear();
    m_holeVertexTest1.clear();
    const int vertexCount = m_holeResolution;
    const float holeRadius = static_cast<float>(m_holeRadius * m_currentEnviron.scale);
    for (int i = 0; i <= vertexCount; ++i)
    {
        double t = Utility::GetPi() * 2 * i / vertexCount;
        m_holeVertex.push_back(DirectX::VertexPositionColor(DirectX::SimpleMath::Vector3(static_cast<float>((holeRadius * cos(t))), static_cast<float>(m_landingHeight), static_cast<float>((holeRadius * -sin(t)))) + aPos, DirectX::Colors::Gray));
        m_holeVertexTest1.push_back(DirectX::VertexPositionNormalColor(DirectX::SimpleMath::Vector3(static_cast<float>((holeRadius * cos(t))), static_cast<float>(m_landingHeight), static_cast<float>((holeRadius * -sin(t)))) + aPos, DirectX::SimpleMath::Vector3::UnitY, DirectX::Colors::Gray));
    }
}

bool Environment::BuildTerrainModel()
{
    m_terrainModel.clear();
    m_terrainModel.resize((m_terrainHeight - 1) * (m_terrainWidth - 1) * 6);
    if (m_terrainModel.size() < 1)
    {
        return false;
    }

    int index = 0;

    for (int j = 0; j < (m_terrainHeight - 1); ++j)
    {
        for (int i = 0; i < (m_terrainWidth - 1); ++i)
        {
            // Get the indexes to the four points of the quad
            int index1 = (m_terrainWidth * j) + i;          // Upper left.
            int index2 = (m_terrainWidth * j) + (i + 1);      // Upper right.
            int index3 = (m_terrainWidth * (j + 1)) + i;      // Bottom left.
            int index4 = (m_terrainWidth * (j + 1)) + (i + 1);  // Bottom right.

            // Now create two triangles for that quad
            // Triangle 1 - Upper left
            m_terrainModel[index].position = m_heightMap[index1].position;
            m_terrainModel[index].normal = m_heightMap[index1].normal;
            ++index;

            // Triangle 1 - Upper right
            m_terrainModel[index].position = m_heightMap[index2].position;
            m_terrainModel[index].normal = m_heightMap[index2].normal;
            ++index;

            // Triangle 1 - Bottom left
            m_terrainModel[index].position = m_heightMap[index3].position;
            m_terrainModel[index].normal = m_heightMap[index3].normal;
            ++index;

            // Triangle 2 - Bottom left
            m_terrainModel[index].position = m_heightMap[index3].position;
            m_terrainModel[index].normal = m_heightMap[index3].normal;
            ++index;

            // Triangle 2 - Upper right.
            m_terrainModel[index].position = m_heightMap[index2].position;
            m_terrainModel[index].normal = m_heightMap[index2].normal;
            ++index;

            // Triangle 2 - Bottom right.
            m_terrainModel[index].position = m_heightMap[index4].position;
            m_terrainModel[index].normal = m_heightMap[index4].normal;
            ++index;
        }
    }

    // to do: m_heightMap.clear(); once testing is complete
    return true;
}

bool Environment::CalculateTerrainNormals()
{
    std::vector<DirectX::SimpleMath::Vector3> normals;
    normals.clear();
    normals.resize((m_terrainHeight - 1) * (m_terrainWidth - 1));
    if (normals.size() < 1)
    {
        return false;
    }
    
    for (int j = 0; j < (m_terrainHeight - 1); ++j)
    {
        for (int i = 0; i < (m_terrainWidth - 1); ++i)
        {
            int index1 = ((j + 1) * m_terrainWidth) + i;      // Bottom left vertex.
            int index2 = ((j + 1) * m_terrainWidth) + (i + 1);  // Bottom right vertex.
            int index3 = (j * m_terrainWidth) + i;          // Upper left vertex.

            // Get three vertices from the face.
            DirectX::SimpleMath::Vector3 vertex1 = m_heightMap[index1].position;
            DirectX::SimpleMath::Vector3 vertex2 = m_heightMap[index2].position;
            DirectX::SimpleMath::Vector3 vertex3 = m_heightMap[index3].position;

            // Calculate the two vectors for this face.
            DirectX::SimpleMath::Vector3 vector1 = vertex1 - vertex3;
            DirectX::SimpleMath::Vector3 vector2 = vertex3 - vertex2;

            int index = (j * (m_terrainWidth - 1)) + i;

            normals[index] = DirectX::XMVector3Cross(vector1, vector2);
            normals[index].Normalize();
        }
    }

    // Now go through all the vertices and take a sum of the face normals that touch this vertex.
    for (int j = 0; j < m_terrainHeight; j++)
    {
        for (int i = 0; i < m_terrainWidth; i++)
        {
            DirectX::SimpleMath::Vector3 sum = DirectX::SimpleMath::Vector3::Zero;

            // Bottom left face.
            if (((i - 1) >= 0) && ((j - 1) >= 0))
            {
                int index = ((j - 1) * (m_terrainWidth - 1)) + (i - 1);
                sum += normals[index];
            }
            // Bottom right face.
            if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
            {
                int index = ((j - 1) * (m_terrainWidth - 1)) + i;
                sum += normals[index];
            }
            // Upper left face.
            if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
            {
                int index = (j * (m_terrainWidth - 1)) + (i - 1);
                sum += normals[index];
            }
            // Upper right face.
            if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
            {
                int index = (j * (m_terrainWidth - 1)) + i;
                sum += normals[index];
            }

            int index = (j * m_terrainWidth) + i;

            sum.Normalize();
            m_heightMap[index].normal = sum;
        }
    }

    return true;
}

void Environment::CreateDataStrings()
{
    std::stringstream inVal;
    inVal.precision(Utility::GetNumericalPrecisionForUI());

    for (int i = 0; i < m_environs.size(); ++i)
    {
        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].airDensity;
        m_environs[i].airDensityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].gravity;
        m_environs[i].gravityStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.x;
        m_environs[i].windXStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.y;
        m_environs[i].windYStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].wind.z;
        m_environs[i].windZStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingFrictionScale;
        m_environs[i].landingFrictionScaleStr = inVal.str();

        inVal.str(std::string());
        inVal << std::fixed << m_environs[i].landingHardnessScale;
        m_environs[i].landingHardnessScaleStr = inVal.str();
    }
}

DirectX::SimpleMath::Vector3 Environment::GetGroundHeight(DirectX::SimpleMath::Vector3 aPos)
{
    DirectX::SimpleMath::Vector3 groundPos = aPos;
    groundPos.y = m_testHeight;
    return groundPos;
}

std::vector<DirectX::VertexPositionColor> Environment::GetTerrainColorVertex()
{
    std::vector<DirectX::VertexPositionColor> vertPosColor;
    vertPosColor.clear();
    vertPosColor.resize(m_terrainModel.size());
    DirectX::XMFLOAT4 terrainColor(1.0, 1.0, 1.0, 1.0); // ToDo: for testing, implement color control

    for (int i = 0; i < vertPosColor.size(); ++i)
    {
        vertPosColor[i].position = m_terrainModel[i].position;
        vertPosColor[i].color = terrainColor;
    }

    return vertPosColor;
}

std::vector<DirectX::VertexPositionNormalColor> Environment::GetTerrainPositionNormalColorVertex()
{
    std::vector<DirectX::VertexPositionNormalColor> vertPosNormColor;
    vertPosNormColor.clear();
    vertPosNormColor.resize(m_terrainModel.size());
    DirectX::XMFLOAT4 terrainColor(1.0, 1.0, 1.0, 1.0); // ToDo: for testing, implement color control

    for (int i = 0; i < vertPosNormColor.size(); ++i)
    {
        vertPosNormColor[i].position = m_terrainModel[i].position;
        vertPosNormColor[i].color = terrainColor;
        vertPosNormColor[i].normal = m_terrainModel[i].normal;
    }

    return vertPosNormColor;
}

float Environment::GetTerrainHeightAtPos(DirectX::XMFLOAT3 aPos) const
{
    /*
    DirectX::SimpleMath::Vector3 vZero = m_terrainModel[0].position;
    DirectX::SimpleMath::Vector3 vOne = m_terrainModel[1].position;
    DirectX::SimpleMath::Vector3 vTwo = m_terrainModel[2].position;
    vZero.y = 0.0;
    vOne.y = 0.0;
    vTwo.y = 0.0;
    float maxDistance0 = DirectX::SimpleMath::Vector3::Distance(vZero, vOne);
    float maxDistance1 = DirectX::SimpleMath::Vector3::Distance(vOne, vTwo);
    float maxDistance2 = DirectX::SimpleMath::Vector3::Distance(vTwo, vZero);
    */
    DirectX::SimpleMath::Vector3 prePos = aPos;
    bool foundHeightBarry = false;
    bool foundHeight = false;
    int index = 0;
    
    int i = 0;
    
    if (aPos.z >= 0.0)
    {
        //i = m_terrainModel.size() / 2;
        //i -= 3;
        i == 2001;
    }
    //for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    //for (int i = 0; i < m_terrainModel.size(); ++i)
    for (i; i < m_terrainModel.size(); ++i)
    {
        /*
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;
        */
        
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[i].position;
        
        if (abs(aPos.x - vertex2.x) < .3 && abs(aPos.z - vertex2.z) < .3)
        {
            foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);
        }
        float f = prePos.x;
        float g = prePos.z;
        DirectX::SimpleMath::Vector3 baryPos = DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, f, g);

        //if (baryPos.x <= 1.0f && baryPos.y <= 1.0f && baryPos.z <= 1.0f)
        if (baryPos.x <= 1.0f && baryPos.x >= 0.0f && baryPos.y <= 1.0f && baryPos.y >= 0.0f && baryPos.z <= 1.0f && baryPos.z >= 0.0f)
        {
            foundHeightBarry = true;
        }
        else
        {
            foundHeightBarry = false;
        }


        if (foundHeight == true && foundHeightBarry == false)
        {
            int testBreak2 = 0;
            testBreak2++;
        }

        if (foundHeight == false && foundHeightBarry == true)
        {
            int testBreak2 = 0;
            testBreak2++;
        }

        if (foundHeight != foundHeightBarry)
        {
            int testBreak2 = 0;
            testBreak2++;
        }

        int testBreak = 0;

        if (foundHeight)
        {
            f = prePos.x;
            g = prePos.z;
            baryPos = DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, f, g);

            testBreak = 0;
            return aPos.y;
        }
    }
    //float errorHeight = -2.0;
    float errorHeight = aPos.y;
    return errorHeight;
}

float Environment::GetTerrainHeightAtPos2(DirectX::XMFLOAT3 aPos) const
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;

        DirectX::SimpleMath::Vector3 testPos = aPos;

        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        if (foundHeight)
        {
            float f = testPos.x;
            float g = testPos.z;
            DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, g, f, testPos);
            DirectX::SimpleMath::Vector3 testPos2 = aPos;
            //return testPos;
            int testBreak = 0;
            ++testBreak;
            return testPos.y;
            //return aPos.y;
        }
    }

    float errorHeight = -2.0;
    return errorHeight;
}

DirectX::SimpleMath::Vector3 Environment::GetTerrainNormal(DirectX::SimpleMath::Vector3 aPos) const
{
    bool foundHeight = false;
    int i = 0;
    if (aPos.z >= 0.0)
    {
        //i = m_terrainModel.size() / 2;
        //i -= 3;
        i == 2001;
    }

    for (i; i < m_terrainModel.size(); ++i)
    {
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[i].position;
        ++i;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[i].position;
        
        DirectX::SimpleMath::Vector3 pos = aPos;
        if (abs(aPos.x - vertex2.x) < .3 && abs(aPos.z - vertex2.z) < .3)
        {
            foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);
        }
        if (foundHeight)
        {
            DirectX::SimpleMath::Vector3 norm1 = m_terrainModel[i-2].normal;
            DirectX::SimpleMath::Vector3 norm2 = m_terrainModel[i-1].normal;
            DirectX::SimpleMath::Vector3 norm3 = m_terrainModel[i].normal;

            float x = (norm1.x + norm2.x + norm3.x) / 3.0f;
            float y = (norm1.y + norm2.y + norm3.y) / 3.0f;
            float z = (norm1.z + norm2.z + norm3.z) / 3.0f;

            DirectX::SimpleMath::Vector3 norm(x, y, z);
 
            norm *= -1;
            norm.Normalize();
            return norm;
            //return aPos;
        }
    }
    
    
    return DirectX::SimpleMath::Vector3::UnitX;
}

DirectX::XMFLOAT3 Environment::GetTerrainPosition(DirectX::XMFLOAT3 aPos)
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;


        DirectX::SimpleMath::Vector3 testPos = aPos;
        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        if (foundHeight)
        {
            float f = testPos.x;
            float g = testPos.z;
            DirectX::SimpleMath::Vector3::Barycentric(vertex1, vertex2, vertex3, f, g, testPos);
            DirectX::SimpleMath::Vector3 testPos2 = aPos;
            //return testPos;
            int testBreak = 0;
            ++testBreak;
            return aPos;
        }
    }
    if (!foundHeight)
    {
        // height cannot be found
        throw std::exception();
    }
    
    return aPos;
}

bool Environment::CheckTerrainTriangleHeight2(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const
{
    /*
    bool foundHeightBarry = false;
    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 baryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    //if (baryPos.x <= 1.0f && baryPos.y <= 1.0f && baryPos.z <= 1.0f)
    if (baryPos.x <= 1.0f && baryPos.x >= 0.0f && baryPos.y <= 1.0f && baryPos.y >= 0.0f && baryPos.z <= 1.0f && baryPos.z >= 0.0f)
    {
        DirectX::SimpleMath::Vector3 vZero = v0;
        DirectX::SimpleMath::Vector3 vOne = v1;
        DirectX::SimpleMath::Vector3 vTwo = v2;

        baryPos.Normalize();

        DirectX::SimpleMath::Vector3 cartPos;
        cartPos = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;
        
        float testSum = baryPos.x + baryPos.y + baryPos.z;
        //float pY = (baryPos.x * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);
        //float pY = ((1 - f - g) * v0.y) + (f * v1.y) + (g * v2.y);
        float pY = ((1 - baryPos.y - baryPos.z) * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);

        //bool testCheck = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);

        DirectX::SimpleMath::Vector3 cartTest = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;


        int testBreak = 0;
        //aPos = cartTest;

        //aPos.y += baryPos.y;
        //aPos.y = aPos.y * cartPos.y;
        //aPos.y = cartPos.y;
        //aPos.y = cartPos.y + v2.y;
        //bool testBool = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);
        foundHeightBarry = true;
    }
    else
    {
        foundHeightBarry = false;
    }

    return foundHeightBarry;
    */

    auto area = [](DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2, DirectX::SimpleMath::Vector3 p3)
    {
        float a = abs(static_cast<float>(p1.x * (p2.z - p3.z) + p2.x * (p3.z - p1.z) + p3.x * (p1.z - p2.z)) * .5);
        return a;
    };

    float AA = area(v0, v1, v2);
    float A1 = area(aPos, v1, v2);
    float A2 = area(v0, aPos, v2);
    float A3 = area(v0, v1, aPos);
    float AASum = A1 + A2 + A3;
    bool isAreaRight = (AA == A1 + A2 + A3);
    float AADifference = AA - AASum;
    
    // Starting position of the ray that is being cast
    DirectX::XMFLOAT3 startVector(aPos.x, 0.0f, aPos.z);
    
    // The direction the ray is being cast
    DirectX::XMFLOAT3 directionVector(0.0f, -1.0f, 0.0f);

    // Calculate the two edges from the three points given
    DirectX::SimpleMath::Vector3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

    // Calculate the normal of the triangle from the two edges // ToDo use cross prod funcs
    DirectX::SimpleMath::Vector3 normal;

    edge1.Cross(edge2, normal);
    normal.Normalize();

    // Find the distance from the origin to the plane.
    float distance = ((-normal.x * v0.x) + (-normal.y * v0.y) + (-normal.z * v0.z));

    // Get the denominator of the equation.
    float denominator = ((normal.x * directionVector.x) + (normal.y * directionVector.y) + (normal.z * directionVector.z));

    // Make sure the result doesn't get too close to zero to prevent divide by zero.
    if (fabs(denominator) < 0.0001f)
    {
        return false;
    }

    // Get the numerator of the equation.
    float numerator = -1.0f * (((normal.x * startVector.x) + (normal.y * startVector.y) + (normal.z * startVector.z)) + distance);

    // Calculate where we intersect the triangle.
    float t = numerator / denominator;

    // Find the intersection vector.
    DirectX::SimpleMath::Vector3 Q;
    Q.x = startVector.x + (directionVector.x * t);
    Q.y = startVector.y + (directionVector.y * t);
    Q.z = startVector.z + (directionVector.z * t);

    // Find the three edges of the triangle.
    DirectX::SimpleMath::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    DirectX::SimpleMath::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

    // Calculate the normal for the first edge.
    DirectX::SimpleMath::Vector3 edgeNormal;
    e1.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    DirectX::SimpleMath::Vector3 temp(Q.x - v1.x, Q.y - v1.y, Q.z - v1.z);

    float determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the second edge
    e2.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v1.x;
    temp.y = Q.y - v1.y;
    temp.z = Q.z - v1.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the third edge.
    e3.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v2.x;
    temp.y = Q.y - v2.y;
    temp.z = Q.z - v2.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }
    
    ////////////////////////////////////////////

    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 vbaryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    DirectX::SimpleMath::Vector3 vZero(0.0, 0.0, 0.0);
    DirectX::SimpleMath::Vector3 vOne(-1.0, 0.0, -1.0);
    DirectX::SimpleMath::Vector3 vTwo(0.0, 0.0, -1.0);
    f = -0.5f;
    g = 0.5f;
    DirectX::SimpleMath::Vector3 testBary = DirectX::SimpleMath::Vector3::Barycentric(vZero, vOne, vTwo, f, g);



    int testBreak = 0;

    ////////////////////////////////////////////

    // Now we have our height.
    aPos.y = vbaryPos.y;
    //aPos.y = Q.y;
    return true;
    
}

bool Environment::CheckTerrainTriangleHeight3(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const
{
    /*
    bool foundHeightBarry = false;
    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 baryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    //if (baryPos.x <= 1.0f && baryPos.y <= 1.0f && baryPos.z <= 1.0f)
    if (baryPos.x <= 1.0f && baryPos.x >= 0.0f && baryPos.y <= 1.0f && baryPos.y >= 0.0f && baryPos.z <= 1.0f && baryPos.z >= 0.0f)
    {
        DirectX::SimpleMath::Vector3 vZero = v0;
        DirectX::SimpleMath::Vector3 vOne = v1;
        DirectX::SimpleMath::Vector3 vTwo = v2;

        baryPos.Normalize();

        DirectX::SimpleMath::Vector3 cartPos;
        cartPos = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;

        float testSum = baryPos.x + baryPos.y + baryPos.z;
        //float pY = (baryPos.x * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);
        //float pY = ((1 - f - g) * v0.y) + (f * v1.y) + (g * v2.y);
        float pY = ((1 - baryPos.y - baryPos.z) * v0.y) + (baryPos.y * v1.y) + (baryPos.z * v2.y);

        //bool testCheck = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);

        DirectX::SimpleMath::Vector3 cartTest = baryPos.x * vZero + baryPos.y * vOne + baryPos.z * vTwo;


        int testBreak = 0;
        //aPos = cartTest;

        //aPos.y += baryPos.y;
        //aPos.y = aPos.y * cartPos.y;
        //aPos.y = cartPos.y;
        //aPos.y = cartPos.y + v2.y;
        //bool testBool = CheckTerrainTriangleHeight2(aPos, v0, v1, v2);
        foundHeightBarry = true;
    }
    else
    {
        foundHeightBarry = false;
    }

    return foundHeightBarry;
    */




    // Starting position of the ray that is being cast
    DirectX::XMFLOAT3 startVector(aPos.x, 0.0f, aPos.z);

    // The direction the ray is being cast
    DirectX::XMFLOAT3 directionVector(0.0f, -1.0f, 0.0f);

    // Calculate the two edges from the three points given
    DirectX::SimpleMath::Vector3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

    // Calculate the normal of the triangle from the two edges // ToDo use cross prod funcs
    DirectX::SimpleMath::Vector3 normal;

    edge1.Cross(edge2, normal);
    normal.Normalize();

    // Find the distance from the origin to the plane.
    float distance = ((-normal.x * v0.x) + (-normal.y * v0.y) + (-normal.z * v0.z));

    // Get the denominator of the equation.
    float denominator = ((normal.x * directionVector.x) + (normal.y * directionVector.y) + (normal.z * directionVector.z));

    // Make sure the result doesn't get too close to zero to prevent divide by zero.
    if (fabs(denominator) < 0.0001f)
    {
        return false;
    }

    // Get the numerator of the equation.
    float numerator = -1.0f * (((normal.x * startVector.x) + (normal.y * startVector.y) + (normal.z * startVector.z)) + distance);

    // Calculate where we intersect the triangle.
    float t = numerator / denominator;

    // Find the intersection vector.
    DirectX::SimpleMath::Vector3 Q;
    Q.x = startVector.x + (directionVector.x * t);
    Q.y = startVector.y + (directionVector.y * t);
    Q.z = startVector.z + (directionVector.z * t);

    // Find the three edges of the triangle.
    DirectX::SimpleMath::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    DirectX::SimpleMath::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

    // Calculate the normal for the first edge.
    DirectX::SimpleMath::Vector3 edgeNormal;
    e1.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    DirectX::SimpleMath::Vector3 temp(Q.x - v1.x, Q.y - v1.y, Q.z - v1.z);

    float determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the second edge
    e2.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v1.x;
    temp.y = Q.y - v1.y;
    temp.z = Q.z - v1.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the third edge.
    e3.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v2.x;
    temp.y = Q.y - v2.y;
    temp.z = Q.z - v2.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    ////////////////////////////////////////////

    float f = aPos.x;
    float g = aPos.z;
    DirectX::SimpleMath::Vector3 vbaryPos = DirectX::SimpleMath::Vector3::Barycentric(v0, v1, v2, f, g);

    DirectX::SimpleMath::Vector3 vZero(0.0, 0.0, 0.0);
    DirectX::SimpleMath::Vector3 vOne(-1.0, 0.0, -1.0);
    DirectX::SimpleMath::Vector3 vTwo(0.0, 0.0, -1.0);
    f = -0.5f;
    g = 0.5f;
    DirectX::SimpleMath::Vector3 testBary = DirectX::SimpleMath::Vector3::Barycentric(vZero, vOne, vTwo, f, g);



    int testBreak = 0;

    ////////////////////////////////////////////

    // Now we have our height.
    aPos.y = Q.y;
    return true;

}

bool Environment::CheckTerrainTriangleHeight4(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const
{
    // Starting position of the ray that is being cast
    DirectX::XMFLOAT3 startVector(aPos.x, 0.0f, aPos.z);

    // The direction the ray is being cast
    DirectX::XMFLOAT3 directionVector(0.0f, -1.0f, 0.0f);

    // Calculate the two edges from the three points given
    DirectX::SimpleMath::Vector3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

    // Calculate the normal of the triangle from the two edges // ToDo use cross prod funcs
    DirectX::SimpleMath::Vector3 normal;

    edge1.Cross(edge2, normal);
    normal.Normalize();

    // Find the distance from the origin to the plane.
    float distance = ((-normal.x * v0.x) + (-normal.y * v0.y) + (-normal.z * v0.z));

    // Get the denominator of the equation.
    float denominator = ((normal.x * directionVector.x) + (normal.y * directionVector.y) + (normal.z * directionVector.z));

    // Make sure the result doesn't get too close to zero to prevent divide by zero.
    if (fabs(denominator) < 0.0001f)
    {
        return false;
    }

    // Get the numerator of the equation.
    float numerator = -1.0f * (((normal.x * startVector.x) + (normal.y * startVector.y) + (normal.z * startVector.z)) + distance);

    // Calculate where we intersect the triangle.
    float t = numerator / denominator;

    // Find the intersection vector.
    DirectX::SimpleMath::Vector3 Q;
    Q.x = startVector.x + (directionVector.x * t);
    Q.y = startVector.y + (directionVector.y * t);
    Q.z = startVector.z + (directionVector.z * t);

    // Find the three edges of the triangle.
    DirectX::SimpleMath::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    DirectX::SimpleMath::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

    // Calculate the normal for the first edge.
    DirectX::SimpleMath::Vector3 edgeNormal;
    e1.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    DirectX::SimpleMath::Vector3 temp(Q.x - v1.x, Q.y - v1.y, Q.z - v1.z);

    float determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the second edge
    e2.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v1.x;
    temp.y = Q.y - v1.y;
    temp.z = Q.z - v1.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the third edge.
    e3.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v2.x;
    temp.y = Q.y - v2.y;
    temp.z = Q.z - v2.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Now we have our height.
    aPos.y = Q.y;
    return true;
}

bool Environment::CheckTerrainTriangleHeight(DirectX::XMFLOAT3& aPos, DirectX::XMFLOAT3 v0, DirectX::XMFLOAT3 v1, DirectX::XMFLOAT3 v2) const
{
    // Starting position of the ray that is being cast
    DirectX::XMFLOAT3 startVector(aPos.x, 0.0f, aPos.z);

    // The direction the ray is being cast
    DirectX::XMFLOAT3 directionVector(0.0f, -1.0f, 0.0f);

    // Calculate the two edges from the three points given
    DirectX::SimpleMath::Vector3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

    // Calculate the normal of the triangle from the two edges // ToDo use cross prod funcs
    DirectX::SimpleMath::Vector3 normal;

    edge1.Cross(edge2, normal);
    normal.Normalize();

    // Find the distance from the origin to the plane.
    float distance = ((-normal.x * v0.x) + (-normal.y * v0.y) + (-normal.z * v0.z));

    // Get the denominator of the equation.
    float denominator = ((normal.x * directionVector.x) + (normal.y * directionVector.y) + (normal.z * directionVector.z));

    // Make sure the result doesn't get too close to zero to prevent divide by zero.
    if (fabs(denominator) < 0.0001f)
    {
        return false;
    }

    // Get the numerator of the equation.
    float numerator = -1.0f * (((normal.x * startVector.x) + (normal.y * startVector.y) + (normal.z * startVector.z)) + distance);

    // Calculate where we intersect the triangle.
    float t = numerator / denominator;

    // Find the intersection vector.
    DirectX::SimpleMath::Vector3 Q;
    Q.x = startVector.x + (directionVector.x * t);
    Q.y = startVector.y + (directionVector.y * t);
    Q.z = startVector.z + (directionVector.z * t);

    // Find the three edges of the triangle.
    DirectX::SimpleMath::Vector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    DirectX::SimpleMath::Vector3 e2(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    DirectX::SimpleMath::Vector3 e3(v0.x - v2.x, v0.y - v2.y, v0.z - v2.z);

    // Calculate the normal for the first edge.
    DirectX::SimpleMath::Vector3 edgeNormal;
    e1.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    DirectX::SimpleMath::Vector3 temp(Q.x - v1.x, Q.y - v1.y, Q.z - v1.z);

    float determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the second edge
    e2.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v1.x;
    temp.y = Q.y - v1.y;
    temp.z = Q.z - v1.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Calculate the normal for the third edge.
    e3.Cross(normal, edgeNormal);

    // Calculate the determinant to see if it is on the inside, outside, or directly on the edge.
    temp.x = Q.x - v2.x;
    temp.y = Q.y - v2.y;
    temp.z = Q.z - v2.z;

    determinant = ((edgeNormal.x * temp.x) + (edgeNormal.y * temp.y) + (edgeNormal.z * temp.z));

    // Check if it is outside.
    if (determinant > 0.001f)
    {
        return false;
    }

    // Now we have our height.
    aPos.y = Q.y;
    return true;
}

// While this could be done once per environment update, future updates could have moment to moment wind changes
double Environment::GetWindDirection() const
{ 
    DirectX::SimpleMath::Vector3 windVec = m_currentEnviron.wind;
    DirectX::SimpleMath::Vector3 zeroDirection(-1.0, 0.0, -1.0);
    double direction = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(DirectX::XMVector3Normalize(windVec), DirectX::XMVector3Normalize(zeroDirection)));
    if (DirectX::XMVectorGetY(DirectX::XMVector3Cross(windVec, zeroDirection)) > 0.0f)
    {
        direction = -direction;
    }

    return direction;
}

bool Environment::InitializeTerrain()
{
    bool result = LoadHeightMap();
    if (!result)
    {
        return false;
    }

    result = CalculateTerrainNormals();
    if (!result)
    {
        return false;
    }

    result = BuildTerrainModel();
    if (!result)
    {
        return false;
    }

    ScaleTerrain();

    return true;
}

void Environment::LoadEnvironmentData()
{
    m_environs.clear();
    m_environs.resize(m_environsAvailable);

    int i = 0;

    m_environs[i].name = "12th";   
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(3.0f, 0.0f, -0.5f); // 12th positon
    SetPosToTerrain(m_environs[i].holePosition);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 3;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-0.3f, 0.0f, -0.3f); 
    SetPosToTerrain(m_environs[i].teePosition);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(7.0f, 0.0f, 0.0f);

    ++i;
    m_environs[i].name = "Breezy";    
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(2.1f, 0.0f, 1.0f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 5;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-2.0f, 0.0f, 0.0f);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(-10.0f, 0.0f, 5.0f);

    ++i;
    m_environs[i].name = "Calm";
    m_environs[i].airDensity = 1.225;
    m_environs[i].gravity = -9.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 1.5f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 3;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 0.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-2.0f, 0.0f, 0.0f);
    m_environs[i].terrainColor = DirectX::Colors::Green;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    /*
    ++i;
    m_environs[i].name = "Non Terrestrial (Alien Golf!!)";    
    m_environs[i].airDensity = 11.2;
    m_environs[i].gravity = -5.8;
    m_environs[i].holePosition = DirectX::SimpleMath::Vector3(0.0f, 0.0f, -1.0f);
    m_environs[i].landingFrictionScale = 1.0;
    m_environs[i].landingHardnessScale = 1.0;
    m_environs[i].par = 5;
    m_environs[i].scale = 0.02;
    m_environs[i].teeDirection = 45.0f;
    m_environs[i].teePosition = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);
    m_environs[i].terrainColor = DirectX::Colors::Blue;
    m_environs[i].wind = DirectX::SimpleMath::Vector3(3.0f, 0.0f, 0.69f);
    */
}

void Environment::LoadFixtureBucket()
{
    bool result;
    DirectX::XMFLOAT3 fixtPos;
    Fixture fixt;

    m_fixtureBucket.clear();
    int i = 0;
    // add FlagStick   
    fixt.idNumber = i;
    // set position to allign with terrain height
    fixtPos = m_currentEnviron.holePosition;
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.fixtureType = FixtureType::FIXTURETYPE_FLAGSTICK;
    fixt.animationVariation = 0.0; // flag stick uses the variable for rotation, keep at 0.0 to keep flag alligned with wind direction
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    // add Tee Box;
    ++i;
    fixt.idNumber = i;
    // set position to allign with terrain height
    fixtPos = m_currentEnviron.teePosition;
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.fixtureType = FixtureType::FIXTURETYPE_TEEBOX;
    fixt.animationVariation = 0.0; // tee box uses this variable for rotation alignment
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);
   
    // Bridges setup
    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = Utility::ToRadians(45.0); // bridge fixture uses this variable for rotation alignment
    fixt.fixtureType = FixtureType::FIXTURETYPE_BRIDGE;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(0.966966, 0.0, -1.08);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ++i;
    fixt.idNumber = i;
    fixt.animationVariation = Utility::ToRadians(45.0); // bridge fixture uses this variable for rotation alignment
    fixt.fixtureType = FixtureType::FIXTURETYPE_BRIDGE;
    // set position to allign with terrain height
    fixtPos = DirectX::XMFLOAT3(3.6, 0.0, 1.5);
    result = SetPosToTerrainWithCheck(fixtPos);
    fixt.position = fixtPos;
    fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
    m_fixtureBucket.push_back(fixt);

    ///////////////////////////////////////////////////////////////////
    // Start placing trees
    ///////////////////////////////////////////////////////////////////

    std::vector<std::pair<DirectX::SimpleMath::Vector3, FixtureType>> posList;
    posList.clear();

    // Right of 12th tee
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(-0.013496, 0.0, 0.329792), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(-0.448391, 0.0, 0.465565), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(0.072614, 0.0, 0.606102), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(0.424795, 0.0, 0.470628), FixtureType::FIXTURETYPE_TREE07));
    // tee side of river trees on the left
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(.754723, 0.0, -1.338842), FixtureType::FIXTURETYPE_TREE06));
    //posList.push_back(std::pair(DirectX::SimpleMath::Vector3(.610597, 0.0, -1.201794), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(.510597, 0.0, -1.601794), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(.565788, 0.0, -1.387254), FixtureType::FIXTURETYPE_TREE09));

    // hole side north of bridge 1
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(1.224668, 0.0, -1.717163), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(1.645601, 0.0, -1.347533), FixtureType::FIXTURETYPE_TREE07));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(1.77, 0.0, -1.703125), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(1.404886, 0.0, -1.970618), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.044166, 0.0, -1.691923), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.265936, 0.0, -1.441618), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.569589, 0.0, -1.624084), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.252271, 0.0, -2.133154), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(1.550634, 0.0, -1.715711), FixtureType::FIXTURETYPE_TREE06));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.979814, 0.0, -1.55047), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.412508, 0.0, -1.676900), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.639267, 0.0, -1.527454), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.969401, 0.0, -1.702111), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.231420, 0.0, -1.314580), FixtureType::FIXTURETYPE_TREE06));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.646301, 0.0, -2.034500), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.157145, 0.0, -1.800968), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.806665, 0.0, -1.724052), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.623988, 0.0, -2.066484), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.326875, 0.0, -1.982778), FixtureType::FIXTURETYPE_TREE07));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.019157, 0.0, -2.004974), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.398504, 0.0, -1.858075), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.113931, 0.0, -2.273377), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.411862, 0.0, -2.356335), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.859739, 0.0, -2.286177), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.508119, 0.0, -2.427412), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.691578, 0.0, -2.668026), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.349274, 0.0, -2.678213), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.207840, 0.0, -2.659259), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.015694, 0.0, -2.721106), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(1.818119, 0.0, -2.490582), FixtureType::FIXTURETYPE_TREE07));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(0.652420, 0.0, -2.145675), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(0.227628, 0.0, -2.629072), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(0.833724, 0.0, -2.643342), FixtureType::FIXTURETYPE_TREE07));

    // hole side north of bridge 1 flowers
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(1.899832, 0.0, -1.347126), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.616034, 0.0, -1.427947), FixtureType::FIXTURETYPE_TREE04));

    // Green backside
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.385089, 0.0, -0.916399), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.424135, 0.0, -1.165066), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.426411, 0.0, -0.392249), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.727715, 0.0, -0.514076), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.559572, 0.0, -0.701670), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.974284, 0.0, -0.441012), FixtureType::FIXTURETYPE_TREE07));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.024924, 0.0, -1.554894), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.311608, 0.0, -1.494708), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.398089, 0.0, -1.826473), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.573008, 0.0, -1.351560), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.615543, 0.0, -1.030336), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.766119, 0.0, -1.808291), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.902010, 0.0, -1.982706), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.346136, 0.0, -2.044371), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.675781, 0.0, -2.007712), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.903988, 0.0, -0.866660), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.880621, 0.0, -1.300454), FixtureType::FIXTURETYPE_TREE06));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.521004, 0.0, -2.682222), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.740935, 0.0, -2.391389), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.835458, 0.0, -2.338160), FixtureType::FIXTURETYPE_TREE07));

    // green backside flowers
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.855693, 0.0, -0.365309), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.060923, 0.0, -0.664320), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.310882, 0.0, -1.144863), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.775608, 0.0, -0.771029), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.826207, 0.0, -1.087361), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.051198, 0.0, -0.861848), FixtureType::FIXTURETYPE_TREE05));
    // right of 13th tee
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.926144, 0.0, 0.399093), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.727443, 0.0, 0.521191), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.266743, 0.0, 1.110051), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.04757, 0.0, 1.212949), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.660378, 0.0, 0.972358), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.900714, 0.0, 0.721872), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.749363, 0.0, 1.748063), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.927847, 0.0, 1.506925), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.446281, 0.0, 1.471418), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.894632, 0.0, 2.039021), FixtureType::FIXTURETYPE_TREE09));

    // right of 13th tee flowers
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.281779, 0.0, 0.968520), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.420808, 0.0, 0.820265), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.548911, 0.0, 0.687745), FixtureType::FIXTURETYPE_TREE04));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.657969, 0.0, 0.539641), FixtureType::FIXTURETYPE_TREE04));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.091162, 0.0, 0.986251), FixtureType::FIXTURETYPE_TREE05));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.213689, 0.0, 0.819230), FixtureType::FIXTURETYPE_TREE05));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.345440, 0.0, 0.630162), FixtureType::FIXTURETYPE_TREE05));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.464575, 0.0, 0.469798), FixtureType::FIXTURETYPE_TREE05));

    // tee side of stream, right of 13th fairway
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.412789, 0.0, 1.526376), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.759058, 0.0, 1.826558), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.827974, 0.0, 2.067842), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.432009, 0.0, 2.153586), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.016640, 0.0, 1.875701), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.740923, 0.0, 2.051027), FixtureType::FIXTURETYPE_TREE07));

    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.297212, 0.0, 2.395568), FixtureType::FIXTURETYPE_TREE07));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(3.628024, 0.0, 2.639376), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(4.493286, 0.0, 2.972688), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.019776, 0.0, 2.499317), FixtureType::FIXTURETYPE_TREE09));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.705706, 0.0, 2.658614), FixtureType::FIXTURETYPE_TREE06));
    posList.push_back(std::pair(DirectX::SimpleMath::Vector3(2.142902, 0.0, 2.035940), FixtureType::FIXTURETYPE_TREE06));


    for (int j = 0; j < posList.size(); ++j)
    {
        ++i;
        fixt.idNumber = i;
        fixt.animationVariation = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 10.0);
        fixt.fixtureType = posList[j].second;
        // set position to allign with terrain height
        fixtPos = posList[j].first;
        result = SetPosToTerrainWithCheck(fixtPos);
        fixt.position = fixtPos;
        fixt.distanceToCamera = DirectX::SimpleMath::Vector3::Distance(fixt.position, m_currentEnviron.teePosition);
        m_fixtureBucket.push_back(fixt);
    }
}

bool Environment::LoadHeightMap()
{
    FILE* filePtr;    
    //char* filename = "heightmap12test2.bmp";
    //char* filename = "heightmapFlat.bmp";
    //char* filename = "heightmapTest.bmp";
    char* filename = "../GolfGame1989/Heightmaps/HeightmapGoldenBell.bmp";
    //char* filename = "../GolfGame1989/Heightmaps/TestMap1.bmp";

    // Open the height map file 
    int error = fopen_s(&filePtr, filename, "rb");
    if (error != 0)
    {
        return false;
    }

    // Read in the file header
    BITMAPFILEHEADER bitmapFileHeader;
    size_t count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    if (count != 1)
    {
        return false;
    }

    // Read in the bitmap info header.
    BITMAPINFOHEADER bitmapInfoHeader;
    count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
    if (count != 1)
    {
        return false;
    }

    // Save the dimensions of the terrain.
    m_terrainWidth = bitmapInfoHeader.biWidth;
    m_terrainHeight = bitmapInfoHeader.biHeight;

    // Calculate the size of the bitmap image data.
    int imageSize = m_terrainWidth * m_terrainHeight * 3 + m_terrainWidth;

    // Allocate memory for the bitmap image data.
    unsigned char* bitmapImage = new unsigned char[imageSize];
    if (!bitmapImage)
    {
        return false;
    }

    // Move to the beginning of the bitmap data.
    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
    
    UINT pitch = m_terrainWidth * 3;
    UINT excessPitch = 0;
    while (double(pitch / 4) != double(pitch) / 4.0)
    {
        pitch++;
        excessPitch++;
    }

    // Read in the bitmap image data.
    count = fread(bitmapImage, 1, pitch * m_terrainHeight, filePtr);
    if (count != imageSize)
    {
        return false;
    }

    std::vector<char> testMap;
    testMap.clear();
    for (int i = 0; i < imageSize; ++i)
    {
        testMap.push_back(bitmapImage[i]);
    }

    // Close the file.
    error = fclose(filePtr);
    if (error != 0)
    {
        return false;
    }

    // Create the structure to hold the height map data.
    m_heightMap.clear();
    m_heightMap.resize(m_terrainWidth * m_terrainHeight);

    // Initialize the position in the image data buffer.
    int k = 0;
    int index;
    unsigned char height;
    
    // Read the image data into the height map.
    for (int j = 0; j < m_terrainWidth; j++)
    {
        for (int i = 0; i < m_terrainHeight; i++)
        {
            height = bitmapImage[k];
            
            // To read values in backwards since bitmap read in is fliped
            index = (m_terrainWidth * (m_terrainHeight - 1 - j)) + i;
            
            m_heightMap[index].position.x = (float)j;
            m_heightMap[index].position.y = (float)height * m_heightScale; // scale height during input
            m_heightMap[index].position.z = (float)i;
            
            k += 3;
        }
        k += excessPitch;
    }

    // Release the bitmap image data.
    delete[] bitmapImage;
    bitmapImage = 0;

    return true;
}

void Environment::ScaleTerrain()
{
    const float scale = .2;
    //const float scale = 10.0;
    const float xTransform = -1.4f;
    const float yTransform = 0.0f;
    const float zTransform = -3.2f;

    for (int i = 0; i < m_heightMap.size(); ++i)
    {
        m_heightMap[i].position.x *= scale;
        m_heightMap[i].position.y *= scale;
        m_heightMap[i].position.z *= scale;

        m_heightMap[i].position.x += xTransform;
        m_heightMap[i].position.y += yTransform;
        m_heightMap[i].position.z += zTransform;
    }
    
    for (int i = 0; i < m_terrainModel.size(); ++i)
    {
        m_terrainModel[i].position.x *= scale;
        m_terrainModel[i].position.y *= scale;
        m_terrainModel[i].position.z *= scale;

        m_terrainModel[i].position.x += xTransform;
        m_terrainModel[i].position.y += yTransform;
        m_terrainModel[i].position.z += zTransform;
    }
}

void Environment::SetLandingHeight(double aLandingHeight)
{
    m_landingHeight = aLandingHeight;
}

void Environment::SetLauchHeight(double aLaunchHeight)
{
    m_launchHeight = aLaunchHeight;
}

bool Environment::SetPosToTerrainWithCheck(DirectX::XMFLOAT3& aPos)
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        int index = i * 3;
        DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
        ++index;
        DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;

        foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);

        if (foundHeight)
        {

            return true;
            //return aPos.y;
        }
    }

    return false;
}

void Environment::SetPosToTerrain(DirectX::XMFLOAT3& aPos)
{
    bool foundHeight = false;
    int index = 0;

    for (int i = 0; i < m_terrainModel.size() / 3; ++i)
    {
        if (foundHeight == false)
        {
            int index = i * 3;
            DirectX::XMFLOAT3 vertex1 = m_terrainModel[index].position;
            ++index;
            DirectX::XMFLOAT3 vertex2 = m_terrainModel[index].position;
            ++index;
            DirectX::XMFLOAT3 vertex3 = m_terrainModel[index].position;

            foundHeight = CheckTerrainTriangleHeight(aPos, vertex1, vertex2, vertex3);
        }
    }
}

void Environment::SortFixtureBucketByDistance()
{
    std::sort(m_fixtureBucket.begin(), m_fixtureBucket.end(),
        [](const auto& i, const auto& j) {return i.distanceToCamera > j.distanceToCamera; });
}

void Environment::UpdateEnvironment(const int aIndex)
{
    m_currentEnviron = m_environs[aIndex];
    m_landingFriction = m_environs[aIndex].landingFrictionScale;
    m_landingHardness = m_environs[aIndex].landingHardnessScale;
    BuildFlagVertex(m_environs[aIndex].holePosition);
    BuildHoleVertex(m_environs[aIndex].holePosition);
}

void Environment::UpdateFixtures(const DirectX::SimpleMath::Vector3 &aPos)
{
    UpdateFixtureDistanceToCamera(aPos);
    SortFixtureBucketByDistance();
}

void Environment::UpdateFixtureDistanceToCamera(const DirectX::SimpleMath::Vector3 &aCameraPos)
{
    for (int i = 0; i < m_fixtureBucket.size(); ++i)
    {
        m_fixtureBucket[i].distanceToCamera = DirectX::SimpleMath::Vector3::Distance(m_fixtureBucket[i].position, aCameraPos);
    }
}
