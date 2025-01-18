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

// ���������scale
glm::vec3 tableScale = glm::vec3(1.0, 1.0, 1.0);
glm::vec3 castleScale = glm::vec3(10.0, 10.0, 10.0);

struct Robot
{
	glm::vec3 translation = glm::vec3(0.0, 0.5, 0.7);	// ���ƻ�����λ��
	float scale_control = 1.0;							// ���ƻ����˴�С
	// �ؽڴ�С
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

	// �ؽڽǺͲ˵�ѡ��ֵ
	enum {
		Torso,			// ����
		Head,			// ͷ��
		RightUpperArm,	// �Ҵ��
		RightLowerArm,	// ��С��
		LeftUpperArm,	// ����
		LeftLowerArm,	// ��С��
		RightUpperLeg,	// �Ҵ���
		RightLowerLeg,	// ��С��
		LeftUpperLeg,	// �����
		LeftLowerLeg,	// ��С��
		left_sword,
	};

	// �ؽڽǴ�С
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
// ��ѡ�е�����
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

// ��ȡ���ɵ�����ģ�ͣ����ڽ�������ʱ�ͷ��ڴ�
std::vector<TriMesh*> meshList;

void draw_sha(TriMesh* m, openGLObject& mesh_object, int sha);
void bindLightAndMaterial(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera);

void drawMesh(glm::mat4 modelMatrix, TriMesh* mesh, openGLObject object) {
	glBindVertexArray(object.vao);
	glUseProgram(object.program);

	// ���ڵ���� * ���ڵ�ֲ��任����
	glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(object.viewLocation, 1, GL_TRUE, &camera->viewMatrix[0][0]);
	glUniformMatrix4fv(object.projectionLocation, 1, GL_TRUE, &camera->projMatrix[0][0]);
	glUniform1i(object.shadowLocation, 0);
	bindLightAndMaterial(mesh, object, light, camera);
	// ����
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());

	// ���ݹ�Դλ�ã�������ӰͶӰ����
	light_position = light->getTranslation();
	float lx = light_position[0];
	float ly = light_position[1];
	float lz = light_position[2];
	glm::mat4 shadowProjMatrix(
		-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
	// ������Ӱ��ģ�ͱ任����
	modelMatrix = shadowProjMatrix * modelMatrix;
	// ���� isShadow ������1��ʾ��ɫ��
	glUniform1i(object.shadowLocation, 1);
	// ���� unifrom �ؼ��ֵľ������ݡ�
	glUniformMatrix4fv(object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	// ����
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

// ����
void torso(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.TORSO_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.TORSO_WIDTH * robot.scale_control, robot.TORSO_HEIGHT * robot.scale_control, robot.TORSO_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, Torso, TorsoObject);
}

// ͷ��
void head(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.5 * robot.HEAD_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.HEAD_WIDTH * robot.scale_control, robot.HEAD_HEIGHT * robot.scale_control, robot.HEAD_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, Head, HeadObject);
}


// ����
void left_upper_arm(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH * robot.scale_control, robot.UPPER_ARM_HEIGHT * robot.scale_control, robot.UPPER_ARM_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, LeftUpperArm, LeftUpperArmObject);
}


// ��С��
void left_lower_arm(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH * robot.scale_control, robot.LOWER_ARM_HEIGHT * robot.scale_control, robot.LOWER_ARM_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, LeftLowerArm, LeftLowerArmObject);
}

// ��
void draw_left_sword(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.1, 0.15));
	instance = glm::scale(instance, glm::vec3(0.03 * robot.scale_control, 0.03 * robot.scale_control, 0.4 * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, left_sword, left_sword_Object);
}

// �Ҵ��
void right_upper_arm(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH * robot.scale_control, robot.UPPER_ARM_HEIGHT * robot.scale_control, robot.UPPER_ARM_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, RightUpperArm, RightUpperArmObject);
}

// ��С��
void right_lower_arm(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_ARM_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH * robot.scale_control, robot.LOWER_ARM_HEIGHT * robot.scale_control, robot.LOWER_ARM_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, RightLowerArm, RightLowerArmObject);
}

// �����
void left_upper_leg(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH * robot.scale_control, robot.UPPER_LEG_HEIGHT * robot.scale_control, robot.UPPER_LEG_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, LeftUpperLeg, LeftUpperLegObject);
}

// ��С��
void left_lower_leg(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH * robot.scale_control, robot.LOWER_LEG_HEIGHT * robot.scale_control, robot.LOWER_LEG_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, LeftLowerLeg, LeftLowerLegObject);
}

// �Ҵ���
void right_upper_leg(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH * robot.scale_control, robot.UPPER_LEG_HEIGHT * robot.scale_control, robot.UPPER_LEG_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, RightUpperLeg, RightUpperLegObject);
}

// ��С��
void right_lower_leg(glm::mat4 modelMatrix)
{
	// ���ڵ�ֲ��任����
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH * robot.scale_control, robot.LOWER_LEG_HEIGHT * robot.scale_control, robot.LOWER_LEG_WIDTH * robot.scale_control));

	// �������Ը������ģ�ͱ任���󣬻��Ƶ�ǰ����
	drawMesh(modelMatrix * instance, RightLowerLeg, RightLowerLegObject);
}


void bindObjectAndData(TriMesh* mesh, openGLObject& object, const std::string& vshader, const std::string& fshader) {

	// ���������������
	glGenVertexArrays(1, &object.vao);  	// ����1�������������
	glBindVertexArray(object.vao);  	// �󶨶����������

	// ��������ʼ�����㻺�����
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
	glBufferData(GL_ARRAY_BUFFER,
		(mesh->getPoints().size() + mesh->getColors().size() + mesh->getNormals().size()) * sizeof(glm::vec3),
		NULL,
		GL_STATIC_DRAW);

	// �޸���TriMesh.cpp�Ĵ���ɺ��ٴ�����ע�ͣ��������ᱨ��
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->getPoints().size() * sizeof(glm::vec3), &mesh->getPoints()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, mesh->getPoints().size() * sizeof(glm::vec3), mesh->getColors().size() * sizeof(glm::vec3), &mesh->getColors()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3), mesh->getNormals().size() * sizeof(glm::vec3), &mesh->getNormals()[0]);

	object.vshader = vshader;
	object.fshader = fshader;
	object.program = InitShader(object.vshader.c_str(), object.fshader.c_str());

	// �Ӷ�����ɫ���г�ʼ�����������
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// �Ӷ�����ɫ���г�ʼ���������ɫ
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->getPoints().size() * sizeof(glm::vec3)));

	// �Ӷ�����ɫ���г�ʼ������ķ�����
	object.nLocation = glGetAttribLocation(object.program, "vNormal");
	glEnableVertexAttribArray(object.nLocation);
	glVertexAttribPointer(object.nLocation, 3,
		GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET((mesh->getPoints().size() + mesh->getColors().size()) * sizeof(glm::vec3)));


	// ��þ���λ��
	object.modelLocation = glGetUniformLocation(object.program, "model");
	object.viewLocation = glGetUniformLocation(object.program, "view");
	object.projectionLocation = glGetUniformLocation(object.program, "projection");

	object.shadowLocation = glGetUniformLocation(object.program, "isShadow");
}


void bindLightAndMaterial(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera) {

	// ���������λ��
	glUniform3fv(glGetUniformLocation(object.program, "eye_position"), 1, &camera->eye[0]);

	// ��������Ĳ���
	glm::vec4 meshAmbient = mesh->getAmbient();
	glm::vec4 meshDiffuse = mesh->getDiffuse();
	glm::vec4 meshSpecular = mesh->getSpecular();
	float meshShininess = mesh->getShininess();
	glUniform4fv(glGetUniformLocation(object.program, "material.ambient"), 1, &meshAmbient[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.diffuse"), 1, &meshDiffuse[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.specular"), 1, &meshSpecular[0]);
	glUniform1f(glGetUniformLocation(object.program, "material.shininess"), meshShininess);

	// ���ݹ�Դ��Ϣ
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
	// ��ȡ��ɫ����ʹ��
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";
	// �����������תλ��
	mesh->setTranslation(glm::vec3(0.0, 0.45, -0.85));
	mesh->setRotation(glm::vec3(0.0, 0.0, 0.0));
	mesh->setScale(glm::vec3(1.0, 1.0, 1.0));
	// ���ò���
	mesh->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // ������
	mesh->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // ������
	mesh->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // ���淴��
	mesh->setShininess(1.0); //�߹�ϵ��
	// ������Ķ������ݴ���
	bindObjectAndData(mesh, mesh_object, vshader, fshader);
}

void init()
{
	std::string vshader, fshader;
	// ��ȡ��ɫ����ʹ��
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";

	// ���ù�Դλ��
	light->setTranslation(glm::vec3(-5.0, 15.0, 10.0));
	light->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0)); // ������
	light->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0)); // ������
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // ���淴��
	light->setAttenuation(1.0, 0.045, 0.0075); // ˥��ϵ��

	// ������������
	// ��ȡ����ģ��
	table->setNormalize(true);
	table->readObj("./assets/table.obj");
	// �����������תλ��
	table->setTranslation(glm::vec3(0.0, 1.6, -0.05));
	table->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	table->setScale(glm::vec3(0.6, 0.6 , 0.6));
	// �ӵ�painter��
	painter->addMesh(table, "mesh_a", "./assets/table.png", vshader, fshader); 	// ָ����������ɫ��
	// ���Ǵ������������һ�������ڣ�Ϊ�˳������ʱ����Щ�����ͷ�
	meshList.push_back(table);

	// ��ȡС�ʵ�ģ��
	wawa->setNormalize(true);
	wawa->readObj("./assets/wawa.obj");
	// �����������תλ��
	wawa->setTranslation(glm::vec3(0.0, 1.1, 0.0));
	wawa->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	wawa->setScale(glm::vec3(1.0, 1.0, 1.0));
	// �ӵ�painter��
	painter->addMesh(wawa, "mesh_b5", "./assets/wawa.png", vshader, fshader); 	// ָ����������ɫ��
	// ���Ǵ������������һ�������ڣ�Ϊ�˳������ʱ����Щ�����ͷ�
	meshList.push_back(wawa);

	// ��ȡ֧����ģ��
	hold_square->setNormalize(true);
	hold_square->readObj("./assets/menger.obj");
	// �����������תλ��
	hold_square->setTranslation(glm::vec3(-0.85, 0.30, -1.1));
	hold_square->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	hold_square->setScale(glm::vec3(1.2, 1.2, 1.2));
	// �ӵ�painter��
	painter->addMesh(hold_square, "mesh_b6", "./assets/menger.png", vshader, fshader); 	// ָ����������ɫ��
	// ���Ǵ������������һ�������ڣ�Ϊ�˳������ʱ����Щ�����ͷ�
	meshList.push_back(table);	
	
	// ��ȡgirlģ��
	girl->setNormalize(true);
	//sword->readObj("./assets/chr_sword/chr_sword.obj");
	//sword->readObj("./assets/cat/cat.obj");
	girl->readObj("./assets/girl_knight.obj");
	// �����������תλ��
	girl->setTranslation(glm::vec3(0.82, 0.34, 0.0));
	girl->setRotation(glm::vec3(0.0, -90.0, 0.0));
	girl->setScale(glm::vec3(1.3, 1.3, 1.3));
	// ���ò���
	girl->setAmbient(glm::vec4(0.60f, 0.60f, 0.60f, 1.0f)); // ������
	girl->setDiffuse(glm::vec4(1.700f, 1.700f, 1.700f, 1.0f)); // ������
	girl->setSpecular(glm::vec4(0.000f, 0.000f, 0.000f, 1.0f)); // ���淴��
	girl->setShininess(1.0f); //�߹�ϵ��
	painter->addMesh(girl, "mesh_b7", "./assets/girl_knight.png", vshader, fshader);
	meshList.push_back(girl);


	// ��ȡƽԭģ��
	plain->setNormalize(true);
	plain->readObj("./assets/monu3.obj");
	// �����������תλ��
	plain->setTranslation(glm::vec3(1.0, 3.4, -2.0));
	plain->setRotation(glm::vec3(0.0, 0.0, 0.0));
	plain->setScale(glm::vec3(14.0, 14.0, 14.0));
	// �ӵ�painter��
	//painter->addMesh(table, "mesh_a", "./assets/table.png", vshader, fshader); 	// ָ����������ɫ��
	plain_painter->addMesh(plain, "mesh_b8", "./assets/monu3.png", vshader, fshader); 	// ָ����������ɫ��
	// ���Ǵ������������һ�������ڣ�Ϊ�˳������ʱ����Щ�����ͷ�
	meshList.push_back(plain);

	/**/
	// ���Ǵ������������һ�������ڣ�Ϊ�˳������ʱ����Щ�����ͷ�
	
	// ��ȡdinosaurģ��
	dinosaur->setNormalize(true);
	dinosaur->readObj("./assets/dinosaur.obj");
	// �����������תλ��
	dinosaur->setTranslation(glm::vec3(-0.85, 0.30, 0.0));
	dinosaur->setRotation(glm::vec3(0.0, 0.0, 0.0));
	dinosaur->setScale(glm::vec3(1.2, 1.2, 1.2));
	// ���ò���
	dinosaur->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0)); // ������
	dinosaur->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0)); // ������
	dinosaur->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // ���淴��
	dinosaur->setShininess(100.0); //�߹�ϵ��
	// �ӵ�painter��
	painter->addMesh(dinosaur, "mesh_c", "./assets/dinosaur.png", vshader, fshader);
	// ���Ǵ������������һ�������ڣ�Ϊ�˳������ʱ����Щ�����ͷ�
	meshList.push_back(dinosaur);
	
	// ����Phong����ģ�͵�����
	mesh_init();

	fshader = "shaders/fshader_1.glsl";
	// ��������Ĵ�С����ʼ����ת��λ�ƶ�Ϊ0��
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


	// ������Ķ������ݴ���
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
	// ���ƹ�������
	glBindVertexArray(mesh_object.vao);
	glUseProgram(mesh_object.program);
	modelMatrix = mesh->getModelMatrix();
	glUniformMatrix4fv(mesh_object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(mesh_object.viewLocation, 1, GL_TRUE, &camera->viewMatrix[0][0]);
	glUniformMatrix4fv(mesh_object.projectionLocation, 1, GL_TRUE, &camera->projMatrix[0][0]);
	// ��isShadow����Ϊa�����ݴ���Ĳ�������
	glUniform1i(mesh_object.shadowLocation, a);
	bindLightAndMaterial(mesh, mesh_object, light, camera);
	glDrawArrays(GL_TRIANGLES, 0, mesh->getPoints().size());
}

void draw_sha(TriMesh* m, openGLObject& mesh_object, int sha) {
	// ���ݹ�Դλ�ã�������ӰͶӰ����
	light_position = light->getTranslation();
	float lx = light_position[0];
	float ly = light_position[1];
	float lz = light_position[2];
	glm::mat4 shadowProjMatrix(
		-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
	// ������Ӱ��ģ�ͱ任����
	glm::mat4 modelMatrix = m->getModelMatrix();
	modelMatrix = shadowProjMatrix * modelMatrix;
	// ���� isShadow ������3��ʾ��ɫ��
	glUniform1i(mesh_object.shadowLocation, sha);
	// ���� unifrom �ؼ��ֵľ������ݡ�
	glUniformMatrix4fv(mesh_object.modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	// ����
	glDrawArrays(GL_TRIANGLES, 0, m->getPoints().size());
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ����������
	camera->updateCamera();
	camera->viewMatrix = camera->getViewMatrix();
	camera->projMatrix = camera->getProjectionMatrix(true);

	// ����һϵ�д���������
	painter->drawMeshes(light, camera, 1);	// ��Ӳ��Ӱ
	plain_painter->drawMeshes(light, camera, -1);	// û��Ӳ��Ӱ;

	// ���ƹ���ģ�ͼ���Ӱ
	draw_obj(mesh, mesh_object, 0);
	draw_sha(mesh, mesh_object, 3);

	//std::cout << robot.translation.x << " " << robot.translation.y << ' ' << robot.translation.z << std::endl;

	// ����ı任����
	glm::mat4 modelMatrix;
	modelMatrix = glm::mat4(1.0);
	// ���ֱ任�����ջ
	MatrixStack mstack;
	// �����˵�λ��
	float x, y, z;
	x = robot.translation.x;
	y = robot.translation.y;
	z = robot.translation.z;

	// ���ɣ���������ϣ�������˵�����ֻ��Y����ת������ֻ������RotateY��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Torso]), glm::vec3(0.0, 1.0, 0.0));
	torso(modelMatrix);

	mstack.push(modelMatrix); // �������ɱ任����
	// ͷ������������ϣ�������˵�ͷ��ֻ��Y����ת������ֻ������RotateY��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Head]), glm::vec3(0.0, 1.0, 0.0));
	head(modelMatrix);
	modelMatrix = mstack.pop(); // �ָ����ɱ任����

	// =========== ��� ===========
	mstack.push(modelMatrix);   // �������ɱ任����
	// ���ۣ���������ϣ�������˵�����ֻ��X����ת������ֻ������RotateX������ͬ��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5 * robot.TORSO_WIDTH - 0.5 * robot.UPPER_ARM_WIDTH, robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperArm]), glm::vec3(1.0, 0.0, 0.0));
	left_upper_arm(modelMatrix);
	// @TODO: ��С��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerArm]), glm::vec3(1.0, 0.0, 0.0));
	left_lower_arm(modelMatrix);
	// ��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.left_sword]), glm::vec3(0.0, 1.0, 0.0));
	draw_left_sword(modelMatrix);
	modelMatrix = mstack.pop(); // �ָ����ɱ任����

	// =========== �ұ� ===========
	mstack.push(modelMatrix);   // �������ɱ任����
	// @TODO: �Ҵ��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5 * robot.TORSO_WIDTH + 0.5 * robot.UPPER_ARM_WIDTH, robot.TORSO_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperArm]), glm::vec3(1.0, .0, 0.0));
	right_upper_arm(modelMatrix);
	// @TODO: ��С��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_ARM_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerArm]), glm::vec3(1.0, 0.0, 0.0));
	right_lower_arm(modelMatrix);
	modelMatrix = mstack.pop(); // �ָ����ɱ任����

	// =========== ���� ===========
	mstack.push(modelMatrix);   // �������ɱ任����
	// @TODO: �����
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.35 * robot.TORSO_WIDTH, 0.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftUpperLeg]), glm::vec3(1.0, 0.0, 0.0));
	left_upper_leg(modelMatrix);
	// @TODO: ��С��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_LEG_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLowerLeg]), glm::vec3(1.0, 0.0, 0.0));
	left_lower_leg(modelMatrix);
	modelMatrix = mstack.pop(); // �ָ����ɱ任����

	// =========== ���� ===========
	mstack.push(modelMatrix);   // �������ɱ任����
	// @TODO: �Ҵ���
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.35 * robot.TORSO_WIDTH, 0.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightUpperLeg]), glm::vec3(1.0, 0.0, 0.0));
	right_upper_leg(modelMatrix);
	// @TODO: ��С��
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -robot.UPPER_LEG_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLowerLeg]), glm::vec3(1.0, 0.0, 0.0));
	right_lower_leg(modelMatrix);
	modelMatrix = mstack.pop(); // �ָ����ɱ任����
}

bool crashJudge(glm::vec3 & pos,int flag)
{
	bool is_OverEdge = false;
	// x��Χ�Ƿ�Խ��
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
	// z��Χ�Ƿ�Խ��
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

	std::cout << "=========================��������==============================" << std::endl << std::endl;

	//std::cout << "��������" << std::endl;
	std::cout <<
		"			[���ڿ���]" << std::endl <<
		"		ESC:			�˳�" << std::endl <<
		std::endl <<

		//"			[�����˿���]" << std::endl <<
		"			[�����˿���ѡ��]" << std::endl <<
		"		\/|:		λ��" << std::endl <<
		"		~/`:		��" << std::endl <<
		"		1:			����" << std::endl <<
		"		2:			ͷ��" << std::endl <<
		"		3:			����" << std::endl <<
		"		4:			��С��" << std::endl <<
		"		5:			�Ҵ��" << std::endl <<
		"		6:			��С��" << std::endl <<
		"		7:			�����" << std::endl <<
		"		8:			��С��" << std::endl <<
		"		9:			�Ҵ���" << std::endl <<
		"		0:			��С��" << std::endl << std::endl <<
		"			[�����˲���]" << std::endl <<
		"		n/N:			������ת�Ƕ�" << std::endl <<
		"		m/M:			��С��ת�Ƕ�" << std::endl <<
		std::endl <<

		"			[ģ���л�]" << std::endl <<
		"		v/V:			�л�Ϊ����" << std::endl <<
		"		b/B:			�л�Ϊţ" << std::endl <<
		"		g/G:			�л�Ϊ����" << std::endl <<
		"		h/H:			�л�ΪƤ����" << std::endl <<
		std::endl <<

		"			[�������ѡ��]" << std::endl <<
		"		u/U:			����С�ʵ�" << std::endl <<
		"		i/i:			����Ů��" << std::endl <<
		"		o/O:			���ƿ���" << std::endl <<
		"		\/|:			���ƻ�����" << std::endl <<
		"		p/P:			����ģ��" << std::endl << std::endl <<
		"			[�������]" << std::endl <<
		"		left:			����" << std::endl <<
		"		right:			����" << std::endl <<
		"		up:			ǰ��" << std::endl <<
		"		down:			����" << std::endl <<
		"		+/=:			���" << std::endl <<
		"		-/_:			��С" << std::endl <<
		"		r/R:			˳ʱ����ת/��ʱ����ת" << std::endl <<
		std::endl <<

		"			[�������]" << std::endl <<
		"		j/J:			˳ʱ����ת/��ʱ����ת" << std::endl <<
		"		k/K:			�����ӽ�/�����ӽ�" << std::endl <<
		"		SPACE:			�������" << std::endl <<
		std::endl <<

		"			[��Դ����]" << std::endl <<
		"		x/X:			��Դ����/��Դ����" << std::endl <<
		"		y/Y:			��Դ����/��Դ����" << std::endl <<
		"		z/Z:			��Դǰ��/��Դ����" << std::endl <<
		std::endl;
}

TriMesh* obj = new TriMesh();
bool is_robot = false;
// ������Ӧ����
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
			// ͨ��������ת������
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
			// ѡ��չʾ����Ч��������
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
			// ѡ����Ҫ���Ƶ�����
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
			//std::cout << "��ʼ���� " << std::endl;
			//std::cout << is_robot << std::endl;
			//std::cout << robot.translation.x << " " << robot.translation.y << ' ' << robot.translation.z << std::endl;
			if (is_robot) {
				std::cout << "�����ǻ��������� " << std::endl;
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

	// �ͷ��ڴ�
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
	// ��ʼ��GLFW�⣬������Ӧ�ó�����õĵ�һ��GLFW����
	glfwInit();

	// ����GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	//�����ַ���ʽ
#pragma execution_character_set("utf-8");
	// ���ô�������
	GLFWwindow* window = glfwCreateWindow(1400, 1400, "2022150153_�޳���_��ĩ����ҵ", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// �����κ�OpenGL�ĺ���֮ǰ��ʼ��GLAD
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
	// ���������Ϣ
	printHelp();
	// ������Ȳ���
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		display();
		// ������ɫ���� �Լ� �����û�д���ʲô�¼�������������롢����ƶ��ȣ�
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanData();


	return 0;
}

// ÿ�����ڸı��С��GLFW�������������������Ӧ�Ĳ������㴦��
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}