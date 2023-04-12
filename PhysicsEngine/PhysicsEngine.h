#pragma once
#include <cmath>
#include <limits>
#include <vector>
#include <format>
#include <string>
#include <map>

using std::vector;
using std::string;
using std::map;

#define PI 3.1415926;

namespace PhysicsEngine {

	class Vector3 {
	public:
		float x;
		float y;
		float z;

	public:
		Vector3() : x(0.0f), y(0.0f), z(0.0f) {	}
		Vector3(float pX, float pY, float pZ) : x(pX), y(pY), z(pZ) {	}

		void Normalize() {
			//t len = std::sqrt(x * x + y * y + z * z);
			float len = sqrtf(x * x + y * y + z * z);
			if (len != 0.0f)
			{
				x /= len;
				y /= len;
				z /= len;
			}
		}

		bool IsNull() {
			return x == 0.0f && y == 0.0f && z == 0.0f ? true : false;
		}

	public:
		Vector3 operator -(Vector3 rhs)
		{
			return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
		}

		Vector3 operator +(Vector3 rhs)
		{
			return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
		}
		Vector3 operator *(float value)
		{
			return Vector3(x * value, y * value, z * value);
		}

		bool operator ==(Vector3 rhs)
		{
			return x == rhs.x && y == rhs.y && z == rhs.z;
		}

		float Magnitude()
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		static float DotProduct(Vector3 a, Vector3 b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		static Vector3 CrossProduct(Vector3 a, Vector3 b)
		{
			float cx = a.y * b.z - a.z * b.y;
			float cy = a.z * b.x - a.x * b.z;
			float cz = a.x * b.y - a.y * b.x;

			return Vector3(cx, cy, cz);
		}

		static float Distance(Vector3 a, Vector3 b)
		{
			float num = a.x - b.x;
			float num2 = a.y - b.y;
			float num3 = a.z - b.z;
			return sqrtf(num * num + num2 * num2 + num3 * num3);
		}

		static Vector3 Project(Vector3 vector, Vector3 onNormal)
		{
			float num = DotProduct(onNormal, onNormal);
			if (num < std::numeric_limits<float>::epsilon())
			{
				return Vector3(0.0f, 0.0f, 0.0f);
			}

			float num2 = DotProduct(vector, onNormal);
			return Vector3(onNormal.x * num2 / num, onNormal.y * num2 / num, onNormal.z * num2 / num);
		}
	};

	class Triangle
	{
	public:
		string name;
		vector<Vector3> vertices;
		Vector3 crossProduct = Vector3(0, 0, 0);
		Vector3 centroid = Vector3(0, 0, 0);

		Triangle(Vector3 a, Vector3 b, Vector3 c)
		{
			vertices.push_back(a);
			vertices.push_back(b);
			vertices.push_back(c);
			CalculateCrossProduct();
			CalculateCentroid();
		}

		Triangle() {};

		Vector3 GetCrossProduct() { return crossProduct; }
		Vector3 GetCentroid() { return centroid; }

	private:

		void CalculateCrossProduct()
		{
			Vector3 a = vertices[0] - vertices[1];
			Vector3 b = vertices[1] - vertices[2];

			crossProduct = Vector3::CrossProduct(a, b);
			crossProduct.Normalize();
		}

		void CalculateCentroid()
		{
			Vector3 temp = vertices[0] + vertices[1] + vertices[2];
			temp.x /= 3;
			temp.y /= 3;
			temp.z /= 3;

			centroid = temp;
		}
	};


	class AdjacentMeshInfo
	{
	public:
		string meshName;
		vector<int> adjacentTriangleIndices;
	};

	class Mesh
	{
	public:
		string name;
		vector<Triangle> listTriangle;
		void AddTriangle(Triangle triangle)
		{
			listTriangle.push_back(triangle);
		}
	};

	class GridInfo
	{
	public:
		string gridIndex;
		vector<AdjacentMeshInfo> listAdjacentMeshInfo;
	};

	class CollisionInfo
	{
	public:
		CollisionInfo() {};

		string fromMeshName;
		Triangle fromTriangle;
		Vector3 penetrationNormal;
		float penetrationDepth = 0.0f;

		bool IsCollided()
		{
			return penetrationDepth != 0.0f;
		}
	};

	enum DotProductType : int
	{
		SAME_DIRACTION = 1,
		OPPOSITE_DIRECTION = -1,
		PERPENDICULAR = 0,
		NONE = -2,
	};

	class CollisionTestUtil
	{
	public:
		static DotProductType GetDotProductTypeBetween(Vector3 a, Vector3 b)
		{
			float value = Vector3::DotProduct(a, b);

			// if (value == 0.0f) �� ���ϰ� �Ǹ� 0.00001f ���� ��� ���� perendicular������ ���Ǻο� Ÿ�� ������ 
			// 0.0f ���ٴ� ũ�� 0.001f ���� ���� ���� perendicular �� �����Ѵ�
			if (0.0f < value && value < 0.001f)
			{
				return DotProductType::PERPENDICULAR;
			}
			else if (value < 0.0f)
			{
				return DotProductType::OPPOSITE_DIRECTION;
			}
			else if (value > 0.0f)
			{
				return DotProductType::SAME_DIRACTION;
			}

			return DotProductType::NONE;
		}

		static CollisionInfo CheckCollision_BetweenTriangleSphere(Triangle triangle, Vector3 target, float radius)
		{
			Vector3 first = triangle.vertices[0];
			Vector3 second = triangle.vertices[1];
			Vector3 third = triangle.vertices[2];

			//DEBUG!! 
			{
				if (triangle.name == "Ramp_Mesh_Triangle(96)")
				{
					string DEBUG_POINT = " DEBUG";
				}
			}

			/*----------------------------------------------------------------------------------------
				�Ʒ� �Լ� ȣ���� �ּ�ģ ������ ���� Seperating Axe �˻縦 ���� �ʾƵ�
				CalculateOverlap_BetweenPointSphere, IsSphereHangingOnLineOfTriangle �Լ����� �ɷ�����.
				Ȥ�� ���߿� ����� �۵� �� �ϸ� �ּ� Ǯ��
				History : ���� Unity prefab�� �̻��� vertex a,b,c �� �� ���ο� �ִ� mesh�� ������ ���� �ִ�.
				�̰�� seperating axe �˻縦 ���ָ� �ϴ� �ɷ�����..
			------------------------------------------------------------------------------------------*/
			// Ȯ���� Seperating Axe ã������ �浹 �˻��� �ʿ䰡 ���� �浹���� ���� �����̴�.
			if (FindSeperatingAxe_BetweenTriangleSphere(triangle.vertices, target, radius))
			{
				return CollisionInfo();
			}

			//������ �浹 ��� ���� - �� ���� ���� �÷��̾ ���� �հ� ���������� �ϸ� ������ �浹�� �߻��ؼ� �� �ǳ������� �����̵� ���� �߻�
			// History - Terrain�� ��û �����ϰ� ���������� ��쿡�� �÷��̾ ����Ű �̵��� ���� �� Normal�� �ݴ�������� ���� ���� �հ� ������ �� �־ �ּ� ó��.
			if (IsTargetLocatedOppositeSideFromTriangleNormal(triangle.GetCrossProduct(), first, target))
			{
				return CollisionInfo();
			}

			if (IsPointInTriangle(first, second, third, target))
			{
				float overlap = CalculateOverlap_BetweenPointSphere(triangle.GetCrossProduct(), first, target, radius);
				if (overlap > 0.0f)
				{
					CollisionInfo collisionInfo;
					collisionInfo.penetrationDepth = overlap;
					collisionInfo.penetrationNormal = triangle.GetCrossProduct();
					return collisionInfo;
				}
			}
			else
			{
				// Sphere�� ���ο� �����ִ� ��쿡�� sphere.radius - line ������ �Ÿ��� penetraion depth�� �Ǿ�� �Ѵ�.
				{
					float overlap = IsSphereHangingOnLineOfTriangle(triangle, target, radius);
					if (overlap > 0.0f)
					{
						CollisionInfo collisionInfo;
						collisionInfo.penetrationDepth = overlap;
						collisionInfo.penetrationNormal = triangle.GetCrossProduct();
						return collisionInfo;
					}
				}
			}

			// �浹 �� �� ���
			return CollisionInfo();
		}

		static bool IsCollidedFromSlope(CollisionInfo collisionInfo)
		{
			if (
				abs(collisionInfo.penetrationNormal.y) > 0.01f &&
				(abs(collisionInfo.penetrationNormal.x) > 0.001f || abs(collisionInfo.penetrationNormal.z) > 0.001f)
				)
			{
				return true;
			}
			else
			{
				return false;
			}
		}


		static CollisionInfo ResolveSlope(CollisionInfo collisionInfo)
		{
			// ��ü�� ������ �浹�ؼ� �߻��� Normal �� �� �� �� �� ���ο��� �߻��ϴ� �浹�̴� �̲��� ������ ���� ���� ���� ó�� ���ش�
			// �� �ϰ� �� ���� �浹�� ������ �ؼ� ��� �ε����� ���̱� ������ y > 0.0f �� ������ ��Ҵ�
			// HISTORY - 0.01f�� �������� �� ���� �̼��� slope�� �����ϱ� ����
			if (IsCollidedFromSlope(collisionInfo))
			{
				Vector3 penetraionNormal = collisionInfo.penetrationNormal;

				// new Vector3(0f, 1f, 0f)�� �ص� �ǰ� new Vector3(0f, temp.y, 0f) �� �ص� ���� ���� ���;��Ѵ�

				//float angle = Vector3::AngleBetween(new Vector3(0.0f, 1.0f, 0.0f), new Vector3(penetraionNormal.x, penetraionNormal.y, penetraionNormal.z)); // In c#
				//float angle = atan2(diff.x, diff.y, diff.z); // In c++
				Vector3 upVector = Vector3(0.0f, 1.0f, 0.0f);
				float angleInDegree = std::acos(Vector3::DotProduct(upVector, penetraionNormal) / (upVector.Magnitude()) * penetraionNormal.Magnitude());

				//float angleInDegree = angle * 180 / PI;
				float anotherAngleInDegree = 90.0f - angleInDegree;

				// �ﰢ���� ��Ģ => a / sin(�ݴ��� ��) = b / sin(�ݴ��� ��) = c / sin (�ݴ��� ��)
				// (���� c �� ���� / Sin(90)) == (penetrationDepth / sin(�ݴ��� ��))
				// Sin(90)�� �׻� 1�̴� ���� ������ => ���� c �� ���� == penetrationDepth / sin(�ݴ��� ��)  
				// float ������ ���� = (collisionInfo.penetrationDepth / sin(anotherAngleInDegree)); 
				// sin() �Լ��� degree�� �ƴ� radian�� ���ڷ� �����Ƿ� Mathf.Deg2Rad �� �����ش�. 
				float newDepth = (collisionInfo.penetrationDepth / std::asin(anotherAngleInDegree)); // Sin(90��)�� ������ 1�̴� ���� 

				CollisionInfo resolved;
				resolved.penetrationDepth = abs(newDepth);
				resolved.penetrationNormal = Vector3(0.0f, 1.0f, 0.0f);
				resolved.fromMeshName = collisionInfo.fromMeshName;
				resolved.fromTriangle = collisionInfo.fromTriangle;

				// ��� ������ ũ�⺸�� Ŭ �� ����...?
				// newDepth �� ������ �����̹Ƿ� �ﰢ���� �� ��� line�� ���̵� �������� Ŭ ���� ����.
				if (collisionInfo.penetrationDepth > newDepth)
				{
					//!!MATHMETICAL ERROR!!!
				}

				return resolved;
			}

			return collisionInfo;
		}

		// Vector3 A�� B �� projectTo(normal) �������� overlap ����� �ϴµ� ������ overlap���� üũ
		// normal -> A B�� ������(forward)
		// normal -> B A�� ������(backward)
		static float CalculateOverlap_BetweenPointSphere(Vector3 projectTo, Vector3 targetA, Vector3 targetB, float radius)
		{
			// ���� 
			// crossProduct�� target ������ �Ÿ��� ���Ѵ�
			// crossProduct�� vertex ������ �Ÿ��� ���Ѵ�(cross product �� vertex[0, 1, 2] ������ Dot product �� ��� ����)
			float distanceToTargetA = Vector3::DotProduct(projectTo, targetA);
			float distanceToTargetB = Vector3::DotProduct(projectTo, targetB);

			// �� ������ �Ÿ��� Sphere.radius ������ �Ÿ����� ������ �浹�� ����!
			float distanceBetween = abs(distanceToTargetA - distanceToTargetB);

			// �Ÿ��� radius ���� ū ��� Overlap�� ����
			if (radius < distanceBetween)
			{
				return 0.0f;
			}

			return radius - distanceBetween;
		}

	private:
		static bool IsTargetLocatedOppositeSideFromTriangleNormal(Vector3 normal, Vector3 targetA, Vector3 targetB)
		{
			float distanceToTargetA = Vector3::DotProduct(normal, targetA);
			float distanceToTargetB = Vector3::DotProduct(normal, targetB);

			return distanceToTargetB < distanceToTargetA ? true : false;
		}

		static float IsSphereHangingOnLineOfTriangle(Triangle t, Vector3 target, float radius)
		{
			int totalCount = t.vertices.size();

			for (int i = 0; i < 3; i++)
			{
				Vector3 A = t.vertices[i];
				Vector3 B = t.vertices[(i + 1) % totalCount];
				Vector3 normal = B - A; // vertex a => vertex b ������ ���� normal

				// Normal �� ���� �� - vertices[i] �������� Target �� - vertices[i] ������ϰ�
				// Perpendicular ���� �� + vertices[i] ������Ѵ�.
				{
					Vector3 newTarget = target - A;
					Vector3 perpendicular = Vector3::Project(newTarget, normal);
					Vector3 newPern = perpendicular + A;

					float A_Dot = Vector3::DotProduct(normal, A);
					float B_Dot = Vector3::DotProduct(normal, B);
					float pernDot = Vector3::DotProduct(normal, newPern);


					// History - B_Dot = 0.98468, pernDot = 0.98455 �� �� ������ �� �浹�� �� �� ó�� ���̴µ�
					// B_Dot < pernDot ���꿡 ���ؼ� return 0f�� �ǰ� �־ float decial�� 3��° �ڸ����� ���� ���� ���� ������ ����
					//A_Dot = (float)System.Math.Round(A_Dot, 3);
					//B_Dot = (float)System.Math.Round(B_Dot, 3);
					//pernDot = (float)System.Math.Round(pernDot, 3);

					// 90���� �̷�� ���� pernDot�� A�� B ���� ���̿� ���� ���
					if (pernDot < A_Dot || B_Dot < pernDot)
					{
						continue;
					}

					float distance = Vector3::Distance(newPern, target);
					if (distance < radius && (radius - distance) > 0.0001f)
					{
						return radius - distance;
					}
				}
			}

			return 0.0f;
		}


		static bool IsPointInTriangle(Vector3 A, Vector3 B, Vector3 C, Vector3 P)
		{
			// ���� ���� [A-B, B-C, C-A]�� ���� ����[A-P, B-P, C-P] �� ���� ������ ����Ű���� üũ
			// ��� ���� ���Ͱ� ���� ������ ����Ų�ٸ� �� P�� Triangle �ȿ� �ִ�
			if (SameSide(P, A, B, C) && SameSide(P, B, A, C) && SameSide(P, C, A, B))
			{
				return true;
			}

			// �ٸ� ������ �ϳ��� ������ Vector P�� Triangle �ȿ� ���� �ʴ�
			return false;
		}

		// �� ���� ���Ͱ� ���� ������ ����Ű���� üũ
		static bool SameSide(Vector3 p1, Vector3 p2, Vector3 A, Vector3 B)
		{
			Vector3 cp1 = Vector3::CrossProduct(B - A, p1 - A); // ���� ���� 1
			Vector3 cp2 = Vector3::CrossProduct(B - A, p2 - A); // ���� ���� 2

			// ���� ���� 1�� 2�� Dot product�� 0 �̻��̸� ���� ����
			if (Vector3::DotProduct(cp1, cp2) >= 0.0f)
			{
				return true;
			}

			// �����̸� �ٸ� ����
			return false;
		}

		static bool FindSeperatingAxe_BetweenTriangleSphere(vector<Vector3>& vertices, Vector3 target, float radius)
		{
			for (int i = 0; i < 3; i++)
			{
				Vector3 normal = target - vertices[i];
				normal.Normalize();

				float targetDotProduct = Vector3::DotProduct(normal, target);

				float max = std::numeric_limits<float>::min();
				float min = std::numeric_limits<float>::max();
				for (int j = 0; j < 3; j++)
				{
					float dotProduct = Vector3::DotProduct(normal, vertices[j]);

					if (dotProduct > max)
					{
						max = dotProduct;
					}

					if (dotProduct < min)
					{
						min = dotProduct;
					}
				}

				if (min > (targetDotProduct + radius) || max < (targetDotProduct - radius))
				{
					return true;
				}
			}

			return false;
		}
	};


	class NavigationMeshAgent
	{
	private:
		int GRID_SIZE;

	public:
		map<string, Mesh> dictMesh;
		map<string, GridInfo> dictGridInfo;

		void SetGridSize(int gridSize) { GRID_SIZE = gridSize; }

		// !!DEPRECATED!!
		void AddTriangle(string meshName, Triangle triangle)
		{
			dictMesh[meshName].AddTriangle(triangle);
		}

		void AddGridInfo(GridInfo gridInfo)
		{
			dictGridInfo[gridInfo.gridIndex] = gridInfo;
		}

		void AddMesh(Mesh mesh)
		{
			if (dictMesh.count(mesh.name))
			{
				//!!ERROR
			}

			dictMesh[mesh.name] = mesh;
		}

		// Version 2 - ��ġ�� ������� GridIndex�� �˾Ƴ��� �ش� Grid�� ������ �ٸ� Grid �鿡 ���ϴ� Mesh �� ������ Triangle �鸸 üũ�Ѵ�.
		// Dependency 1 - GetBestCollisionInfo_From_Terrain : �浹�� ���� �� ���� ��Ȯ�� ������ ó��
		// Dependency 2 - GetBestCollisionInfo_From_Object : �浹�� ���� �� DotProductType ���� ���� ��Ȯ�� �����鸸 ó��
		Vector3 ResolveCollision(Vector3 position, float radius, bool ignoreSlope = true)
		{
			if (!dictMesh.count("Terrain"))
			{
				return position;
			}

			int row = (int)(position.z / GRID_SIZE);
			int column = (int)(position.x / GRID_SIZE);
			string gridIndex = std::format("{}_{}", row, column);

			GridInfo gridInfo = dictGridInfo[gridIndex];

			vector<CollisionInfo> listTotalCollisionInfo;

			for (AdjacentMeshInfo& adjacentMeshInfo : gridInfo.listAdjacentMeshInfo)
			{
				string meshName = adjacentMeshInfo.meshName;
				Mesh mesh = dictMesh[meshName];


				bool isTerrain = meshName.starts_with("Terrain");

				if (isTerrain)
				{
					//mesh, adjacentTriangleInfo, position, radius

					CollisionInfo terrainCollisionInfo = GetBestCollisionInfo_From_Terrain(mesh, adjacentMeshInfo.adjacentTriangleIndices, position, radius);

					if (terrainCollisionInfo.IsCollided())
					{
						listTotalCollisionInfo.push_back(terrainCollisionInfo);
					}
				}
				else
				{
					vector<CollisionInfo> listCollisionInfo = GetBestCollisionInfo_From_Object(mesh, adjacentMeshInfo.adjacentTriangleIndices, position, radius);

					for (CollisionInfo& collisionInfo : listCollisionInfo)
					{
						listTotalCollisionInfo.push_back(collisionInfo);
					}
				}
			}

			Vector3 addPosition;

			for (CollisionInfo& collisionInfo : listTotalCollisionInfo)
			{
				addPosition = addPosition + (collisionInfo.penetrationNormal * collisionInfo.penetrationDepth);
			}

			return position + addPosition;
		}

		// Terrain ���� ���� ��Ȯ�� �浹 ���� �� ���� ��Ȯ�� ���� �� 1�� �����Ѵ�.
	private:
		CollisionInfo GetBestCollisionInfo_From_Terrain(Mesh mesh, vector<int> triangleIndicesToCheck, Vector3 position, float radius)
		{
			vector<CollisionInfo> listCollisionInfo;

			for (int triangleIndex : triangleIndicesToCheck)
			{
				Triangle triangle = mesh.listTriangle[triangleIndex];

				CollisionInfo collisionInfo = CollisionTestUtil::CheckCollision_BetweenTriangleSphere(triangle, position, radius);
				if (collisionInfo.IsCollided())
				{
					collisionInfo.fromMeshName = mesh.name;
					collisionInfo.fromTriangle = triangle;

					listCollisionInfo.push_back(collisionInfo);
				}
			}

			if (listCollisionInfo.size() == 0)
			{
				return CollisionInfo();
			}

			float maxPenetraion = std::numeric_limits<float>::min();
			CollisionInfo finalCollisionInfo; // ���� ��Ȯ�� �浹 ����

			// ���� ��Ȯ�� �浹�� penetrationDepth �� ���� ū �浹�̴�.
			for (CollisionInfo& collisionInfo : listCollisionInfo)
			{
				if (maxPenetraion < collisionInfo.penetrationDepth)
				{
					maxPenetraion = collisionInfo.penetrationDepth;
					finalCollisionInfo = collisionInfo;
				}
			}

			// ����� �̲������� �����Ѵٸ� �浹�� ���ο��� �߻��ߴ��� üũ �� resolve ���ش�
			if (CollisionTestUtil::IsCollidedFromSlope(finalCollisionInfo))
			{
				finalCollisionInfo = CollisionTestUtil::ResolveSlope(finalCollisionInfo);
			}

			return finalCollisionInfo;
		}

		// Object ���� �浹�� ���� �� DotProductType ���� ���� ��Ȯ�� �浹 ������ �����Ѵ�.
		vector<CollisionInfo> GetBestCollisionInfo_From_Object(Mesh mesh, vector<int>& triangleIndicesToCheck, Vector3 position, float radius)
		{
			// Ver1 - Vector3(1,0,0) �� DotProduct�� ����ؼ� ����, 0, ����� �ش��ϴ� normal���� �浹 ������ List �� ������.(����� ���⿡�� �߻��� �浹 ���� �� ���� ��Ȯ�� �͸� �����ϱ� ����
			map<DotProductType, vector<CollisionInfo>> dict_direction_to_listCollision;

			Vector3 criteria;

			// �浹�� ��� ������ DotProductType �������� ������.
			for (int triangleIndex : triangleIndicesToCheck)
			{
				Triangle triangle = mesh.listTriangle[triangleIndex];

				CollisionInfo collisionInfo = CollisionTestUtil::CheckCollision_BetweenTriangleSphere(triangle, position, radius);
				if (collisionInfo.IsCollided())
				{
					collisionInfo.fromMeshName = mesh.name;
					collisionInfo.fromTriangle = triangle;

					// ���⼭ 2���� ����� ������� �ִ�.
					// ��� 1 : �浹�� Normal ����� �� ����� ������� �׷����� ���� �� Normal ���� ���� ��Ȯ�� Collision Info�� ã�� ����
					// ���� : �ǹ� ������ ������ ������ �������� �� �� jittering�� �߻����� �ʴ´� �޺� �պ��� �浹�� ��� ó�����ֱ� ����, ��� 2�� �ϸ� jiterring �߻�
					//
					// ��� 2 : Normal ���� ������� Mesh���� ���� ��Ȯ�� Collision Info �� �ϳ��� ã��
					// ��� 2 ���� : ���� ��ü�� �浹�ؼ� ���� �� jittering �߻�, �𼭸��� �����ٰ� �������� �� ���ӵ��� �ٴ� ������ �ִ�

					// HISTORY - ��� DotProductType::NONE �� key�� �־��� ������ �ϳ��� Mesh���� �߻��� �� �ϳ��� Collision Info�� üũ���ຸ��
					//DotProductType type = DotProductType::NONE;

					if (criteria.IsNull())
					{
						criteria = collisionInfo.penetrationNormal;
					}

					DotProductType type = CollisionTestUtil::GetDotProductTypeBetween(criteria, collisionInfo.penetrationNormal);

					dict_direction_to_listCollision[type].push_back(collisionInfo);
				}
			}

			vector<CollisionInfo> listCollisionInfo;

			// ���� ����(normal)���� �浹�� ���� �� ���� ��Ȯ�� ������ ã�´�.(���� ����(normal)���� �浹�� �߻� ���� ��� ���� ��Ȯ�� �浹 ������ penetrationDepth�� ���� ū ���� �� �̴�)
			for (auto& [_, vecCollisionInfo] : dict_direction_to_listCollision)
			{
				float maxPenetraion = std::numeric_limits<float>::min();
				CollisionInfo finalCollisionInfo; // ���� ��Ȯ�� �浹 ����

				// �� normal�� �浹 ���� �� ���� ��Ȯ�� ������ ã�´�.
				for (CollisionInfo& collisionInfo : vecCollisionInfo)
				{
					if (maxPenetraion < collisionInfo.penetrationDepth)
					{
						maxPenetraion = collisionInfo.penetrationDepth;
						finalCollisionInfo = collisionInfo;
					}
				}

				// ���� �̲����� ������ �ʿ��ϴٸ� ó�����ش�.
				if (CollisionTestUtil::IsCollidedFromSlope(finalCollisionInfo))
				{
					finalCollisionInfo = CollisionTestUtil::ResolveSlope(finalCollisionInfo);
				}

				// �� normal�� �浹 ���� �� ���� ��Ȯ�� ������ ���� ���� �־��ش�.
				listCollisionInfo.push_back(finalCollisionInfo);

			}

			return listCollisionInfo;
		}
	};
}

