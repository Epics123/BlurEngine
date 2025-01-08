#include "Model.h"
#include "Utility.h"

#include <glm/gtc/packing.hpp>

namespace EngineCore
{

	EngineCore::Vertex16Bit Vertex::To16BitVertex()
	{
		Vertex16Bit Out;

		Out.PosX = glm::packHalf1x16(Position.x);
		Out.PosY = glm::packHalf1x16(Position.y);
		Out.PosZ = glm::packHalf1x16(Position.z);

		Out.ColorX = glm::packHalf1x16(Color.x);
		Out.ColorY = glm::packHalf1x16(Color.y);
		Out.ColorZ = glm::packHalf1x16(Color.z);

		Out.NormalX = glm::packHalf1x16(Normal.x);
		Out.NormalY = glm::packHalf1x16(Normal.y);
		Out.NormalZ = glm::packHalf1x16(Normal.z);

		Out.TangentX = glm::packHalf1x16(Tangent.x);
		Out.TangentY = glm::packHalf1x16(Tangent.y);
		Out.TangentZ = glm::packHalf1x16(Tangent.z);

		Out.BiTangentX = glm::packHalf1x16(BiTangent.x);
		Out.BiTangentY = glm::packHalf1x16(BiTangent.y);
		Out.BiTangentZ = glm::packHalf1x16(BiTangent.z);

		Out.TexcoordU = glm::packHalf1x16(TexCoord.x);
		Out.TexcoordV = glm::packHalf1x16(TexCoord.y);

		return Out;
	}

}