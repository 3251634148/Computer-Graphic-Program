#include "Angel.h"
#include "TriMesh.h"
#include "Camera.h"
#include "MeshPainter.h"

#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>

int WIDTH = 600;
int HEIGHT = 600;

int mainWindow;

class MatrixStack {
	int		_index;
	int		_size;
	glm::mat4* _matrices;

public:
	MatrixStack(int numMatrices = 100) :_index(0), _size(numMatrices)
	{
		_matrices = new glm::mat4[numMatrices];
	}

	~MatrixStack()
	{
		delete[]_matrices;
	}

	void push(const glm::mat4& m) {
		assert(_index + 1 < _size);
		_matrices[_index++] = m;
	}

	glm::mat4& pop() {
		assert(_index - 1 >= 0);
		_index--;
		return _matrices[_index];
	}
};


#define White	glm::vec3(1.0, 1.0, 1.0)
#define Yellow	glm::vec3(1.0, 1.0, 0.0)
#define Green	glm::vec3(0.0, 1.0, 0.0)
#define Cyan	glm::vec3(0.0, 1.0, 1.0)
#define Magenta	glm::vec3(1.0, 0.0, 1.0)
#define Red		glm::vec3(1.0, 0.0, 0.0)
#define Black	glm::vec3(0.0, 0.0, 0.0)
#define Blue	glm::vec3(0.0, 0.0, 1.0)
#define Brown	glm::vec3(0.5, 0.5, 0.5)
#define Purple  glm::vec3(0.5, 0.0, 0.5)
#define sky     glm::vec3(0.4, 0.77, 1.0)

// 设置物体的scale
glm::vec3 tableScale = glm::vec3(1.0, 1.0, 1.0);
glm::vec3 castleScale = glm::vec3(10.0, 10.0, 10.0);

struct Robot
{
	glm::vec3 translation = glm::vec3(0.0, 0.5, 0.7);	// 控制机器人位置
	float scale_control = 1.0;							// 控制机器人大小
	// 关节大小
	float TORSO_HEIGHT = 4.0 * 0.1;
	float TORSO_WIDTH = 2.5 * 0.1;
	float UPPER_ARM_HEIGHT = 2.5 * 0.1;
	float LOWER_ARM_HEIGHT = 1.8 * 0.1;
	float UPPER_ARM_WIDTH = 0.8 * 0.1;
	float LOWER_ARM_WIDTH = 0.5 * 0.1;
	float UPPER_LEG_HEIGHT = 2.8 * 0.1;
	float LOWER_LEG_HEIGHT = 2.2 * 0.1;
	float UPPER_LEG_WIDTH = 1.0 * 0.1;
	float LOWER_LEG_WIDTH = 0.5 * 0.1;
	float HEAD_HEIGHT = 1.8 * 0.1;
	float HEAD_WIDTH = 1.5 * 0.1;

	// 关节角和菜单选项值
	enum {
		Torso,			// 躯干
		Head,			// 头部
		RightUpperArm,	// 右大臂
		RightLowerArm,	// 右小臂
		LeftUpperArm,	// 左大臂
		LeftLowerArm,	// 左小臂
		RightUpperLeg,	// 右大腿
		RightLowerLeg,	// 右小腿
		LeftUpperLeg,	// 左大腿
		LeftLowerLeg,	// 左小腿
		left_sword,
	};

	// 关节角大小
	GLfloat theta[10] = {
		0.0,    // Torso
		0.0,    // Head
		0.0,    // RightUpperArm
		0.0,    // RightLowerArm
		0.0,    // LeftUpperArm
		0.0,    // LeftLowerArm
		0.0,    // RightUpperLeg
		0.0,    // RightLowerLeg
		0.0,    // LeftUpperLeg
		0.0     // LeftLowerLeg
	};
};
Robot robot;
// 被选中的物体
int Selected_mesh = robot.Torso;

TriMesh* Torso = new TriMesh();
TriMesh* Head = new TriMesh();
TriMesh* RightUpperArm = new TriMesh();
TriMesh* RightLowerArm = new TriMesh();
TriMesh* LeftUpperArm = new TriMesh();
TriMesh* LeftLowerArm = new TriMesh();
TriMesh* RightUpperLeg = new TriMesh();
TriMesh* RightLowerLeg = new TriMesh();
TriMesh* LeftUpperLeg = new TriMesh();
TriMesh* LeftLowerLeg = new TriMesh();
TriMesh* left_sword = new TriMesh();

openGLObject TorsoObject;
openGLObject HeadObject;
openGLObject RightUpperArmObject;
openGLObject RightLowerArmObject;
openGLObject LeftUpperArmObject;
openGLObject LeftLowerArmObject;
openGLObject RightUpperLegObject;
openGLObject RightLowerLegObject;
openGLObject LeftUpperLegObject;
openGLObject LeftLowerLegObject;
openGLObject left_sword_Object;

openGLObject mesh_object;
openGLObject plane_object;

TriMesh* plain = new TriMesh();
TriMesh* table = new TriMesh();
TriMesh* girl = new TriMesh();
TriMesh* dinosaur = new TriMesh();
TriMesh* plane = new TriMesh();
TriMesh* mesh = new TriMesh();
TriMesh* wawa = new TriMesh();
TriMesh* hold_square = new TriMesh();

Camera* camera = new Camera();
Light* light = new Light();
MeshPainter* painter = new MeshPainter();
MeshPainter* plain_painter = new MeshPainter();

glm::vec3 light_position;

float move_step_size = 0.2;

// 获取生成的所有模型，用于结束程序时释放内存
std::vector<TriMesh*> meshList;

void draw_sha(TriMesh* m, openGLObject& mesh_object, int sha);
void bindLightAndMaterial(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera);

void drawMesh(glm::mat4 modelMatrix, TriMesh* mesh, openGLObject object) {
	glBindVertexArray(object.vao);
	glUseProgram(object.program);

	// 父节点矩阵 * 本节点局部变换矩阵
	glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(object.viewLocation, 1, GL_TRUE, &camera->viewMatrix[0][0]);
	glUniformMatrix4fv(object.projectionLocation, 1, GL_TRUE, &camera->projMatrix[0][0]);
	glUniform1i(object.shadowLocation, 0);
	bindLightAndMaterial(mesh, object, light, camera);
	// 绘制
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());

	// 根据光源位置，计算阴影投影矩阵
	light_position = light->getTranslation();
	float lx = light_position[0];
	float ly = light_position[1];
	float lz = light_position[2];
	glm::mat4 shadowProjMatrix(
		-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
	// 计算阴影的模型变换矩阵。
	modelMatrix = shadowProjMatrix * modelMatrix;
	// 传递 isShadow 变量，1表示黑色。
	glUniform1i(object.shadowLocation, 1);
	// 传递 unifrom 关键字的矩阵数据。
	glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	// 绘制
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

// 躯体
void torso(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.TORSO_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.TORSO_WIDTH * robot.scale_control, robot.TORSO_HEIGHT * robot.scale_control, robot.TORSO_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, Torso, TorsoObject);
}

// 头部
void head(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.HEAD_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.HEAD_WIDTH * robot.scale_control, robot.HEAD_HEIGHT * robot.scale_control, robot.HEAD_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, Head, HeadObject);
}


// 左大臂
void left_upper_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH * robot.scale_control, robot.UPPER_ARM_HEIGHT * robot.scale_control, robot.UPPER_ARM_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, LeftUpperArm, LeftUpperArmObject);
}


// 左小臂
void left_lower_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH * robot.scale_control, robot.LOWER_ARM_HEIGHT * robot.scale_control, robot.LOWER_ARM_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, LeftLowerArm, LeftLowerArmObject);
}

// 剑
void draw_left_sword(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.1, 0.15));
	instance = glm::scale(instance, glm::vec3(0.03 * robot.scale_control, 0.03 * robot.scale_control, 0.4 * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, left_sword, left_sword_Object);
}

// 右大臂
void right_upper_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH * robot.scale_control, robot.UPPER_ARM_HEIGHT * robot.scale_control, robot.UPPER_ARM_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, RightUpperArm, RightUpperArmObject);
}

// 右小臂
void right_lower_arm(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH * robot.scale_control, robot.LOWER_ARM_HEIGHT * robot.scale_control, robot.LOWER_ARM_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, RightLowerArm, RightLowerArmObject);
}

// 左大腿
void left_upper_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH * robot.scale_control, robot.UPPER_LEG_HEIGHT * robot.scale_control, robot.UPPER_LEG_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, LeftUpperLeg, LeftUpperLegObject);
}

// 左小腿
void left_lower_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH * robot.scale_control, robot.LOWER_LEG_HEIGHT * robot.scale_control, robot.LOWER_LEG_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, LeftLowerLeg, LeftLowerLegObject);
}

// 右大腿
void right_upper_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH * robot.scale_control, robot.UPPER_LEG_HEIGHT * robot.scale_control, robot.UPPER_LEG_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, RightUpperLeg, RightUpperLegObject);
}

// 右小腿
void right_lower_leg(glm::mat4 modelMatrix)
{
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH * robot.scale_control, robot.LOWER_LEG_HEIGHT * robot.scale_control, robot.LOWER_LEG_WIDTH * robot.scale_control));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	drawMesh(modelMatrix * instance, RightLowerLeg, RightLowerLegObject);
}


void bindObjectAndData(TriMesh* mesh, openGLObject& object, const std::string& vshader, const std::string& fshader) {

	// 创建顶点数组对象
	glGenVertexArrays(1, &object.vao);  	// 分配1个顶点数组对象
	glBindVertexArray(object.vao);  	// 绑定顶点数组对象

	// 创建并初始化顶点缓存对象
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
	glBufferData(GL_ARRAY_BUFFER,
		(mesh->getPoints().size() + mesh->getColors().size() + mesh->getNormals().size()) * sizeof(glm::vec3),
		NULL,
		GL_STATIC_DRAW);

	// 修改完TriMesh.cpp的代码成后再打开下面注释，否则程序会报错
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->getPoints().size() * sizeof(glm::vec3), &mesh->getPoints()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, mesh->getPoints().size() * sizeof(glm::vec3), mesh->getColors().size() * sizeof(glm::vec3), &mesh->getColors()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3), mesh->getNormals().size() * sizeof(glm::vec3), &mesh->getNormals()[0]);

	object.vshader = vshader;
	object.fshader = fshader;
	object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());

	// 从顶点着色器中初始化顶点的坐标
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// 从顶点着色器中初始化顶点的颜色
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->getPoints().size() * sizeof(glm::vec3)));

	// 从顶点着色器中初始化顶点的法向量
	object.nLocation = glGetAttribLocation(object.program, "vNormal");
	glEnableVertexAttribArray(object.nLocation);
	glVertexAttribPointer(object.nLocation, 3,
		GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET((mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3)));


	// 获得矩阵位置
	object.modelLocation = glGetUniformLocation(object.program, "model");
	object.viewLocation = glGetUniformLocation(object.program, "view");
	object.projectionLocation = glGetUniformLocation(object.program, "projection");

	object.shadowLocation = glGetUniformLocation(object.program, "isShadow");
}


void bindLightAndMaterial(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera) {

	// 传递相机的位置
	glUniform3fv(glGetUniformLocation(object.program, "eye_position"), 1, &camera->eye[0]);

	// 传递物体的材质
	glm::vec4 meshAmbient = mesh->getAmbient();
	glm::vec4 meshDiffuse = mesh->getDiffuse();
	glm::vec4 meshSpecular = mesh->getSpecular();
	float meshShininess = mesh->getShininess();
	glUniform4fv(glGetUniformLocation(object.program, "material.ambient"), 1, &meshAmbient[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.diffuse"), 1, &meshDiffuse[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.specular"), 1, &meshSpecular[0]);
	glUniform1f(glGetUniformLocation(object.program, "material.shininess"), meshShininess);

	// 传递光源信息
	glm::vec4 lightAmbient = light->getAmbient();
	glm::vec4 lightDiffuse = light->getDiffuse();
	glm::vec4 lightSpecular = light->getSpecular();
	glm::vec3 lightPosition = light->getTranslation();

	glUniform4fv(glGetUniformLocation(object.program, "light.ambient"), 1, &lightAmbient[0]);
	glUniform4fv(glGetUniformLocation(object.program, "light.diffuse"), 1, &lightDiffuse[0]);
	glUniform4fv(glGetUniformLocation(object.program, "light.specular"), 1, &lightSpecular[0]);
	glUniform3fv(glGetUniformLocation(object.program, "light.position"), 1, &lightPosition[0]);

}

void mesh_init() {
	std::string vshader, fshader;
	// 读取着色器并使用
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";
	// 设置物体的旋转位移
	mesh->setTranslation(glm::vec3(0.0, 0.45, -0.85));
	mesh->setRotation(glm::vec3(0.0, 0.0, 0.0));
	mesh->setScale(glm::vec3(1.0, 1.0, 1.0));
	// 设置材质
	mesh->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	mesh->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	mesh->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	mesh->setShininess(1.0); //高光系数
	// 将物体的顶点数据传递
	bindObjectAndData(mesh, mesh_object, vshader, fshader);
}

void init()
{
	std::string vshader, fshader;
	// 读取着色器并使用
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";

	// 设置光源位置
	light->setTranslation(glm::vec3(-5.0, 15.0, 10.0));
	light->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 环境光
	light->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 漫反射
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 镜面反射
	light->setAttenuation(1.0, 0.045, 0.0075); // 衰减系数

	// 设置物体纹理
	// 读取桌子模型
	table->setNormalize(true);
	table->readObj("./assets/table.obj");
	// 设置物体的旋转位移
	table->setTranslation(glm::vec3(0.0, 1.6, -0.05));
	table->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	table->setScale(glm::vec3(0.6, 0.6 , 0.6));
	// 加到painter中
	painter->addMesh(table, "mesh_a", "./assets/table.png", vshader, fshader); 	// 指定纹理与着色器
	// 我们创建的这个加入一个容器内，为了程序结束时将这些数据释放
	meshList.push_back(table);

	// 读取小皇帝模型
	wawa->setNormalize(true);
	wawa->readObj("./assets/wawa.obj");
	// 设置物体的旋转位移
	wawa->setTranslation(glm::vec3(0.0, 1.1, 0.0));
	wawa->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	wawa->setScale(glm::vec3(1.0, 1.0, 1.0));
	// 加到painter中
	painter->addMesh(wawa, "mesh_b5", "./assets/wawa.png", vshader, fshader); 	// 指定纹理与着色器
	// 我们创建的这个加入一个容器内，为了程序结束时将这些数据释放
	meshList.push_back(wawa);

	// 读取支撑物模型
	hold_square->setNormalize(true);
	hold_square->readObj("./assets/menger.obj");
	// 设置物体的旋转位移
	hold_square->setTranslation(glm::vec3(-0.85, 0.30, -1.1));
	hold_square->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	hold_square->setScale(glm::vec3(1.2, 1.2, 1.2));
	// 加到painter中
	painter->addMesh(hold_square, "mesh_b6", "./assets/menger.png", vshader, fshader); 	// 指定纹理与着色器
	// 我们创建的这个加入一个容器内，为了程序结束时将这些数据释放
	meshList.push_back(table);	
	
	// 读取girl模型
	girl->setNormalize(true);
	//sword->readObj("./assets/chr_sword/chr_sword.obj");
	//sword->readObj("./assets/cat/cat.obj");
	girl->readObj("./assets/girl_knight.obj");
	// 设置物体的旋转位移
	girl->setTranslation(glm::vec3(0.82, 0.34, 0.0));
	girl->setRotation(glm::vec3(0.0, -90.0, 0.0));
	girl->setScale(glm::vec3(1.3, 1.3, 1.3));
	// 设置材质
	girl->setAmbient(glm::vec4(0.60f, 0.60f, 0.60f, 1.0f)); // 环境光
	girl->setDiffuse(glm::vec4(1.700f, 1.700f, 1.700f, 1.0f)); // 漫反射
	girl->setSpecular(glm::vec4(0.000f, 0.000f, 0.000f, 1.0f)); // 镜面反射
	girl->setShininess(1.0f); //高光系数
	painter->addMesh(girl, "mesh_b7", "./assets/girl_knight.png", vshader, fshader);
	meshList.push_back(girl);


	// 读取平原模型
	plain->setNormalize(true);
	plain->readObj("./assets/monu3.obj");
	// 设置物体的旋转位移
	plain->setTranslation(glm::vec3(1.0, 3.4, -2.0));
	plain->setRotation(glm::vec3(0.0, 0.0, 0.0));
	plain->setScale(glm::vec3(14.0, 14.0, 14.0));
	// 加到painter中
	//painter->addMesh(table, "mesh_a", "./assets/table.png", vshader, fshader); 	// 指定纹理与着色器
	plain_painter->addMesh(plain, "mesh_b8", "./assets/monu3.png", vshader, fshader); 	// 指定纹理与着色器
	// 我们创建的这个加入一个容器内，为了程序结束时将这些数据释放
	meshList.push_back(plain);

	/**/
	// 我们创建的这个加入一个容器内，为了程序结束时将这些数据释放
	
	// 读取dinosaur模型
	dinosaur->setNormalize(true);
	dinosaur->readObj("./assets/dinosaur.obj");
	// 设置物体的旋转位移
	dinosaur->setTranslation(glm::vec3(-0.85, 0.30, 0.0));
	dinosaur->setRotation(glm::vec3(0.0, 0.0, 0.0));
	dinosaur->setScale(glm::vec3(1.2, 1.2, 1.2));
	// 设置材质
	dinosaur->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 环境光
	dinosaur->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 漫反射
	dinosaur->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	dinosaur->setShininess(100.0); //高光系数
	// 加到painter中
	painter->addMesh(dinosaur, "mesh_c", "./assets/dinosaur.png", vshader, fshader);
	// 我们创建的这个加入一个容器内，为了程序结束时将这些数据释放
	meshList.push_back(dinosaur);
	
	// 传入Phong光照模型的内容
	mesh_init();

	fshader = "shaders/fshader_1.glsl";
	// 设置物体的大小（初始的旋转和位移都为0）
	Torso->setNormalize(true);
	Head->setNormalize(true);
	RightUpperArm->setNormalize(true);
	LeftUpperArm->setNormalize(true);
	RightUpperLeg->setNormalize(true);
	LeftUpperLeg->setNormalize(true);
	RightLowerArm->setNormalize(true);
	LeftLowerArm->setNormalize(true);
	RightLowerLeg->setNormalize(true);
	LeftLowerLeg->setNormalize(true);
	left_sword->setNormalize(true);

	Torso->generateCube(Purple);
	Head->generateCube(Green);
	RightUpperArm->generateCube(Yellow);
	LeftUpperArm->generateCube(Yellow);
	RightUpperLeg->generateCube(Brown);
	LeftUpperLeg->generateCube(Brown);
	RightLowerArm->generateCube(Blue);
	LeftLowerArm->generateCube(Blue);
	RightLowerLeg->generateCube(Cyan);
	LeftLowerLeg->generateCube(Cyan);
	left_sword->generateCube(Red);


	// 将物体的顶点数据传递
	bindObjectAndData(Torso, TorsoObject, vshader, fshader);
	bindObjectAndData(Head, HeadObject, vshader, fshader);
	bindObjectAndData(RightUpperArm, RightUpperArmObject, vshader, fshader);
	bindObjectAndData(LeftUpperArm, LeftUpperArmObject, vshader, fshader);
	bindObjectAndData(RightUpperLeg, RightUpperLegObject, vshader, fshader);
	bindObjectAndData(LeftUpperLeg, LeftUpperLegObject, vshader, fshader);
	bindObjectAndData(RightLowerArm, RightLowerArmObject, vshader, fshader);
	bindObjectAndData(LeftLowerArm, LeftLowerArmObject, vshader, fshader);
	bindObjectAndData(RightLowerLeg, RightLowerLegObject, vshader, fshader);
	bindObjectAndData(LeftLowerLeg, LeftLowerLegObject, vshader, fshader);
	bindObjectAndData(left_sword, left_sword_Object, vshader, fshader);

	//glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 250.0f / 255.0f, 1.0);
	glClearColor(0.4, 0.77, 1.0, 1.0);
}

void draw_obj(TriMesh* mesh, openGLObject& mesh_object, int a) {
	glm::mat4 modelMatrix;
	// 绘制光照物体
	glBindVertexArray(mesh_object.vao);
	glUseProgram(mesh_object.program);
	modelMatrix = mesh->getModelMatrix();
	glUniformMatrix4fv(mesh_object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(mesh_object.viewLocation, 1, GL_TRUE, &camera->viewMatrix[0][0]);
	glUniformMatrix4fv(mesh_object.projectionLocation, 1, GL_TRUE, &camera->projMatrix[0][0]);
	// 将isShadow设置为a，根据传入的参数绘制
	glUniform1i(mesh_object.shadowLocation, a);
	bindLightAndMaterial(mesh, mesh_object, light, camera);
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

void draw_sha(TriMesh* m, openGLObject& mesh_object, int sha) {
	// 根据光源位置，计算阴影投影矩阵
	light_position = light->getTranslation();
	float lx = light_position[0];
	float ly = light_position[1];
	float lz = light_position[2];
	glm::mat4 shadowProjMatrix(
		-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
	// 计算阴影的模型变换矩阵。
	glm::mat4 modelMatrix = m->getModelMatrix();
	modelMatrix = shadowProjMatrix * modelMatrix;
	// 传递 isShadow 变量，3表示黑色。
	glUniform1i(mesh_object.shadowLocation, sha);
	// 传递 unifrom 关键字的矩阵数据。
	glUniformMatrix4fv(mesh_object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	// 绘制
	glDrawArrays(GL_TRIANGLES, 0, m->getPoints().size());
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 相机矩阵计算
	camera->updateCamera();
	camera->viewMatrix = camera->getViewMatrix();
	camera->projMatrix = camera->getProjectionMatrix(true);

	// 绘制一系列带纹理物体
	painter->drawMeshes(light, camera, 1);	// 有硬阴影
	plain_painter->drawMeshes(light, camera, -1);	// 没有硬阴影;

	// 绘制光照模型及阴影
	draw_obj(mesh, mesh_object, 0);
	draw_sha(mesh, mesh_object, 3);

	//std::cout << robot.translation.x << " " << robot.translation.y << ' ' << robot.translation.z << std::endl;

	// 物体的变换矩阵
	glm::mat4 modelMatrix;
	modelMatrix = glm::mat4(1.0);
	// 保持变换矩阵的栈
	MatrixStack mstack;
	// 机器人的位移
	float x, y, z;
	x = robot.translation.x;
	y = robot.translation.y;
	z = robot.translation.z;

	// 躯干（这里我们希望机器人的躯干只绕Y轴旋转，所以只计算了RotateY）
	modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Torso]), glm::vec3(0.0, 1.0, 0.0));
	torso(modelMatrix);

	mstack.push(modelMatrix); // 保存躯干变换矩阵
	// 头部（这里我们希望机器人的头部只绕Y轴旋转，所以只计算了RotateY）
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Head]), glm::vec3(0.0, 1.0, 0.0));
	head(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵

	// =========== 左臂 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// 左大臂（这里我们希望机器人的左大臂只绕X轴旋转，所以只计算了RotateX，后面同理）
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5 * robot.TORSO_WIDTH - 0.5 * robot.UPPER_ARM_WIDTH, robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperArm]), glm::vec3(1.0, 0.0, 0.0));
	left_upper_arm(modelMatrix);
	// @TODO: 左小臂
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerArm]), glm::vec3(1.0, 0.0, 0.0));
	left_lower_arm(modelMatrix);
	// 剑
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.left_sword]), glm::vec3(0.0, 1.0, 0.0));
	draw_left_sword(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵

	// =========== 右臂 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// @TODO: 右大臂
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5 * robot.TORSO_WIDTH + 0.5 * robot.UPPER_ARM_WIDTH, robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperArm]), glm::vec3(1.0, .0, 0.0));
	right_upper_arm(modelMatrix);
	// @TODO: 右小臂
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerArm]), glm::vec3(1.0, 0.0, 0.0));
	right_lower_arm(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵

	// =========== 左腿 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// @TODO: 左大腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.35 * robot.TORSO_WIDTH, 0.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperLeg]), glm::vec3(1.0, 0.0, 0.0));
	left_upper_leg(modelMatrix);
	// @TODO: 左小腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_LEG_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerLeg]), glm::vec3(1.0, 0.0, 0.0));
	left_lower_leg(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵

	// =========== 右腿 ===========
	mstack.push(modelMatrix);   // 保存躯干变换矩阵
	// @TODO: 右大腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.35 * robot.TORSO_WIDTH, 0.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperLeg]), glm::vec3(1.0, 0.0, 0.0));
	right_upper_leg(modelMatrix);
	// @TODO: 右小腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_LEG_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerLeg]), glm::vec3(1.0, 0.0, 0.0));
	right_lower_leg(modelMatrix);
	modelMatrix = mstack.pop(); // 恢复躯干变换矩阵
}

bool crashJudge(glm::vec3 & pos,int flag)
{
	bool is_OverEdge = false;
	// x范围是否越界
	if (flag == 1) {
		double x = pos.x;
		if (x <= -3.2) {
			pos.x = -3.2;
			is_OverEdge = true;
		}
		if (x >= 5.3) {
			pos.x = 5.3;
			is_OverEdge = true;
		}
	}
	// z范围是否越界
	else {
		double z = pos.z;
		if (z >= 2.3) {
			pos.z = 2.3;
			is_OverEdge = true;
		}
	}
	return is_OverEdge;
}

void printHelp()
{

	std::cout << "=========================按键介绍==============================" << std::endl << std::endl;

	//std::cout << "按键介绍" << std::endl;
	std::cout <<
		"			[窗口控制]" << std::endl <<
		"		ESC:			退出" << std::endl <<
		std::endl <<

		//"			[机器人控制]" << std::endl <<
		"			[机器人控制选择]" << std::endl <<
		"		\/|:		位置" << std::endl <<
		"		~/`:		剑" << std::endl <<
		"		1:			躯干" << std::endl <<
		"		2:			头部" << std::endl <<
		"		3:			左大臂" << std::endl <<
		"		4:			左小臂" << std::endl <<
		"		5:			右大臂" << std::endl <<
		"		6:			右小臂" << std::endl <<
		"		7:			左大腿" << std::endl <<
		"		8:			左小腿" << std::endl <<
		"		9:			右大腿" << std::endl <<
		"		0:			右小腿" << std::endl << std::endl <<
		"			[机器人操作]" << std::endl <<
		"		n/N:			增加旋转角度" << std::endl <<
		"		m/M:			减小旋转角度" << std::endl <<
		std::endl <<

		"			[模型切换]" << std::endl <<
		"		v/V:			切换为球体" << std::endl <<
		"		b/B:			切换为牛" << std::endl <<
		"		g/G:			切换为松鼠" << std::endl <<
		"		h/H:			切换为皮卡丘" << std::endl <<
		std::endl <<

		"			[物体控制选择]" << std::endl <<
		"		u/U:			控制小皇帝" << std::endl <<
		"		i/i:			控制女孩" << std::endl <<
		"		o/O:			控制恐龙" << std::endl <<
		"		\/|:			控制机器人" << std::endl <<
		"		p/P:			控制模型" << std::endl << std::endl <<
		"			[物体操作]" << std::endl <<
		"		left:			左移" << std::endl <<
		"		right:			右移" << std::endl <<
		"		up:			前移" << std::endl <<
		"		down:			后移" << std::endl <<
		"		+/=:			变大" << std::endl <<
		"		-/_:			变小" << std::endl <<
		"		r/R:			顺时针旋转/逆时针旋转" << std::endl <<
		std::endl <<

		"			[相机控制]" << std::endl <<
		"		j/J:			顺时针旋转/逆时针旋转" << std::endl <<
		"		k/K:			上移视角/下移视角" << std::endl <<
		"		SPACE:			重置相机" << std::endl <<
		std::endl <<

		"			[光源控制]" << std::endl <<
		"		x/X:			光源左移/光源右移" << std::endl <<
		"		y/Y:			光源下移/光源上移" << std::endl <<
		"		z/Z:			光源前移/光源后移" << std::endl <<
		std::endl;
}

TriMesh* obj = new TriMesh();
bool is_robot = false;
// 键盘响应函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	float tmp;
	glm::vec3 temp;
	glm::vec4 ambient;
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key)
		{
		case GLFW_KEY_ESCAPE: exit(EXIT_SUCCESS); break;
		//case GLFW_KEY_Q: exit(EXIT_SUCCESS); break;
		case GLFW_KEY_GRAVE_ACCENT: Selected_mesh = robot.left_sword; break;
		case GLFW_KEY_1: Selected_mesh = robot.Torso; break;
		case GLFW_KEY_2: Selected_mesh = robot.Head; break;
		case GLFW_KEY_3: Selected_mesh = robot.RightUpperArm; break;
		case GLFW_KEY_4: Selected_mesh = robot.RightLowerArm; break;
		case GLFW_KEY_5: Selected_mesh = robot.LeftUpperArm; break;
		case GLFW_KEY_6: Selected_mesh = robot.LeftLowerArm; break;
		case GLFW_KEY_7: Selected_mesh = robot.RightUpperLeg; break;
		case GLFW_KEY_8: Selected_mesh = robot.RightLowerLeg; break;
		case GLFW_KEY_9: Selected_mesh = robot.LeftUpperLeg; break;
		case GLFW_KEY_0: Selected_mesh = robot.LeftLowerLeg; break;
		case GLFW_KEY_BACKSLASH: is_robot = true; break;
			// 通过按键旋转机器人
		case GLFW_KEY_N:
			robot.theta[Selected_mesh] += 5.0;
			if (robot.theta[Selected_mesh] > 360.0)
				robot.theta[Selected_mesh] -= 360.0;
			break;
		case GLFW_KEY_M:
			robot.theta[Selected_mesh] -= 5.0;
			if (robot.theta[Selected_mesh] < 0.0)
				robot.theta[Selected_mesh] += 360.0;
			break;
			// 选择展示光照效果的物体
		case GLFW_KEY_V:
			mesh = new TriMesh();
			mesh->readOff("./assets/off/sphere.off");
			mesh_init();
			break;
		case GLFW_KEY_B:
			mesh = new TriMesh();
			mesh->readOff("./assets/off/cow.off");
			mesh_init();
			break;
		case GLFW_KEY_G:
			mesh = new TriMesh();
			mesh->readOff("./assets/off/Squirtle.off");
			mesh_init();
			break;
		case GLFW_KEY_H:
			mesh = new TriMesh();
			mesh->readOff("./assets/off/Pikachu.off");
			mesh_init();
			break;
			// 选择需要控制的物体
		case GLFW_KEY_U:
			obj = wawa;
			is_robot = false;
			break;
		case GLFW_KEY_I:
			obj = girl;
			is_robot = false;
			break;
		case GLFW_KEY_O:
			obj = dinosaur;
			is_robot = false;
			break;
		case GLFW_KEY_P:
			obj = mesh;
			is_robot = false;
			break;
		case GLFW_KEY_MINUS:
			if (is_robot) {
				robot.scale_control -= 0.1;
			}
			else {
				temp = obj->getScale();
				temp.x -= 0.2;
				temp.y -= 0.2;
				temp.z -= 0.2;
				obj->setScale(temp);
			}			
			break;
		case GLFW_KEY_EQUAL:
			if (is_robot) {
				robot.scale_control += 0.1;
			}
			else {
				temp = obj->getScale();
				temp.x += 0.2;
				temp.y += 0.2;
				temp.z += 0.2;
				obj->setScale(temp);
			}
			break;
		case GLFW_KEY_R:
			temp = obj->getRotation();
			if (mode == GLFW_MOD_SHIFT) {
				if (is_robot) {
					robot.theta[1] += 90.0;
					if (robot.theta[1] > 360.0)robot.theta[1] -= 360;					
				}
				else 
					temp.y += 90.0;
			}	
			else {
				if (is_robot) {
					robot.theta[1] -= 90.0;
					if (robot.theta[1] < 0.0)robot.theta[1] += 360;					
				}
				else
					temp.y -= 90.0;
			}
			obj->setRotation(temp);
			break;
		case GLFW_KEY_LEFT:
			//std::cout << "开始左移 " << std::endl;
			//std::cout << is_robot << std::endl;
			//std::cout << robot.translation.x << " " << robot.translation.y << ' ' << robot.translation.z << std::endl;
			if (is_robot) {
				std::cout << "现在是机器人左移 " << std::endl;
				temp = robot.translation;
				temp.x -= 0.1;
				crashJudge(temp, 1);
				robot.translation = temp;
			}
			else {
				temp = obj->getTranslation();
				temp.x -= 0.1;
				crashJudge(temp, 1);
				obj->setTranslation(temp);			
				if (obj == wawa) {
					temp = table->getTranslation();
					temp.x -= 0.1;
					crashJudge(temp, 1);
					table->setTranslation(temp);
				}
			}
			
			break;
		case GLFW_KEY_RIGHT:
			if (is_robot) {
				temp = robot.translation;
				temp.x += 0.1;
				crashJudge(temp, 1);
				robot.translation = temp;
			}
			else {
				temp = obj->getTranslation();
				temp.x += 0.1;
				crashJudge(temp, 1);
				obj->setTranslation(temp);
				if (obj == wawa) {
					temp = table->getTranslation();
					temp.x += 0.1;
					crashJudge(temp, 1);
					table->setTranslation(temp);
				}
			}
			break;
		case GLFW_KEY_UP:
			if (is_robot) {
				temp = robot.translation;
				temp.z -= 0.1;
				crashJudge(temp, 2);
				robot.translation = temp;
			}
			else {
				temp = obj->getTranslation();
				temp.z -= 0.1;
				crashJudge(temp, 2);
				obj->setTranslation(temp);
				if (obj == wawa) {
					temp = table->getTranslation();
					temp.z -= 0.1;
					crashJudge(temp, 2);
					table->setTranslation(temp);
				}
			}
			break;
		case GLFW_KEY_DOWN:
			if (is_robot) {
				temp = robot.translation;
				temp.z += 0.1;
				crashJudge(temp, 2);
				robot.translation = temp;
			}
			else {
				temp = obj->getTranslation();
				temp.z += 0.1;
				crashJudge(temp, 2);
				obj->setTranslation(temp);
				if (obj == wawa) {
					temp = table->getTranslation();
					temp.z += 0.1;
					crashJudge(temp, 2);
					table->setTranslation(temp);
				}
			}			
			break;
		case GLFW_KEY_X:
			light_position = light->getTranslation();
			if (mode == GLFW_MOD_SHIFT)
				light_position[0] += move_step_size;
			else
				light_position[0] -= move_step_size;
			light->setTranslation(light_position);
			break;
		case GLFW_KEY_Y:
			light_position = light->getTranslation();
			if (mode == GLFW_MOD_SHIFT)
				light_position[1] += move_step_size;
			else {
				light_position[1] -= move_step_size;
				if (light_position[1] <= 1.0) {
					light_position[1] += move_step_size;
				}
			}
			light->setTranslation(light_position);
			break;
		case GLFW_KEY_Z:
			light_position = light->getTranslation();
			if (mode == GLFW_MOD_SHIFT)
				light_position[2] += move_step_size;
			else
				light_position[2] -= move_step_size;
			light->setTranslation(light_position);
			break;
		default:
			camera->keyboard(window, key, action, mode);
			break;
		}
	}
}


void cleanData() {

	// 释放内存
	delete camera;
	camera = NULL;

	delete light;
	light = NULL;

	painter->cleanMeshes();

	delete painter;
	painter = NULL;

	for (int i = 0; i < meshList.size(); i++) {
		meshList[i]->cleanData();
		delete meshList[i];
	}
	meshList.clear();

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv)
{
	// 初始化GLFW库，必须是应用程序调用的第一个GLFW函数
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	//设置字符格式
#pragma execution_character_set("utf-8");
	// 配置窗口属性
	GLFWwindow* window = glfwCreateWindow(1400, 1400, "2022150153_罗晨曦_期末大作业", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 调用任何OpenGL的函数之前初始化GLAD
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	mesh->setNormalize(true);
	mesh->readOff("./assets/off/Pikachu.off");
	// Init mesh, shaders, buffer
	init();
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		display();
		// 交换颜色缓冲 以及 检查有没有触发什么事件（比如键盘输入、鼠标移动等）
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanData();


	return 0;
}

// 每当窗口改变大小，GLFW会调用这个函数并填充相应的参数供你处理。
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}