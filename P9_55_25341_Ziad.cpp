#include <cstdio>
#include <cmath>
#include <cstdlib>  // For rand()
#include <ctime>
#include <vector>
#include <string>
#include <al.h>
#include <alc.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <glut.h>


// Function to initialize OpenAL
ALCdevice* device;
ALCcontext* context;
ALuint buffer, source;
ALuint bufferBackground, bufferYouDied, bufferYouWin, bufferCollision, bufferTreadmill, bufferSmith, bufferBenchPress, bufferDumbbellRack, bufferChinUp, bufferDeadlift;
ALuint sourceBackground, sourceYouDied, sourceYouWin, sourceCollision, sourceTreadmill, sourceSmith, sourceBenchPress, sourceDumbbellRack, sourceChinUp, sourceDeadlift;

void initOpenAL() {
	device = alcOpenDevice(NULL); // Open default device
	std::cerr << device << std::endl;
	if (!device) {
		std::cerr << "Failed to open audio device." << std::endl;
		return;
	}

	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context)) {
		std::cerr << "Failed to set audio context." << std::endl;
		return;
	}

	// Set up listener properties
	ALfloat listenerPos[] = { 0.0f, 0.0f, 0.0f }; // Listener position
	ALfloat listenerVel[] = { 0.0f, 0.0f, 0.0f }; // Listener velocity
	ALfloat listenerOri[] = { 0.0f, 0.0f, -1.0f,  // Orientation: looking down -Z axis
							  0.0f, 1.0f, 0.0f }; // Up vector: +Y axis

	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);

	// Generate buffer and source
	// Generate buffer and source for background music
	alGenBuffers(1, &bufferBackground);
	alGenSources(1, &sourceBackground);

	// Generate buffer and source for "You Died" sound
	alGenBuffers(1, &bufferYouDied);
	alGenSources(1, &sourceYouDied);

	// Generate buffer and source for "You Win" sound
	alGenBuffers(1, &bufferYouWin);
	alGenSources(1, &sourceYouWin);

	// Generate buffer and source for obstacle collision sound
	alGenBuffers(1, &bufferCollision);
	alGenSources(1, &sourceCollision);

	alGenBuffers(1, &bufferTreadmill);
	alGenSources(1, &sourceTreadmill);

	alGenBuffers(1, &bufferSmith);
	alGenSources(1, &sourceSmith);

	alGenBuffers(1, &bufferBenchPress);
	alGenSources(1, &sourceBenchPress);

	alGenBuffers(1, &bufferDumbbellRack);
	alGenSources(1, &sourceDumbbellRack);

	alGenBuffers(1, &bufferChinUp);
	alGenSources(1, &sourceChinUp);

	alGenBuffers(1, &bufferDeadlift);
	alGenSources(1, &sourceDeadlift);

	alSourcef(source, AL_GAIN, 1.0f); // Set gain to normal volume

	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		std::cerr << "OpenAL Error: " << error << std::endl;
	}
}


// Function to load a .WAV file into OpenAL
struct WAVHeader {
	char riff[4];                // "RIFF"
	int chunkSize;                // File size minus 8 bytes
	char wave[4];                 // "WAVE"
	char fmt[4];                  // "fmt "
	int subChunk1Size;            // Size of the fmt chunk (16 for PCM)
	short audioFormat;            // Audio format (1 for PCM)
	short numChannels;            // Number of channels
	int sampleRate;               // Sample rate (e.g., 44100 Hz)
	int byteRate;                 // Byte rate (SampleRate * NumChannels * BitsPerSample / 8)
	short blockAlign;             // Block alignment
	short bitsPerSample;          // Bits per sample (e.g., 16)
	char data[4];                 // "data"
	int dataSize;                 // Size of the data chunk
};

// Function to load the WAV file and upload to OpenAL
void loadWAVFile(const char* filename, ALuint& buffer) {
	// Open the WAV file
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to open WAV file: " << filename << std::endl;
		return;
	}

	// Read the WAV header
	WAVHeader header;
	file.read(reinterpret_cast<char*>(&header), sizeof(header));

	// Check if the file is in valid WAV format
	if (header.riff[0] != 'R' || header.riff[1] != 'I' || header.riff[2] != 'F' || header.riff[3] != 'F') {
		std::cerr << "Invalid WAV file format for: " << filename << std::endl;
		file.close();
		return;
	}

	// Read audio data
	char* data = new char[header.dataSize];
	file.read(data, header.dataSize);
	file.close();

	// Determine the audio format (Mono or Stereo)
	ALenum format;
	if (header.numChannels == 1) {
		format = (header.bitsPerSample == 16) ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	}
	else if (header.numChannels == 2) {
		format = (header.bitsPerSample == 16) ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
	}
	else {
		std::cerr << "Unsupported WAV format for: " << filename << std::endl;
		delete[] data;
		return;
	}

	// Generate the buffer if not already created
	if (buffer == 0) {
		alGenBuffers(1, &buffer);
	}

	// Load the audio data into the OpenAL buffer
	alBufferData(buffer, format, data, header.dataSize, header.sampleRate);
	delete[] data; // Free audio data
}


void loadSoundInBackground() {
	loadWAVFile("Stereo Madness.wav", bufferBackground);
	loadWAVFile("Dark Souls.wav", bufferYouDied);
	loadWAVFile("Super Mario Win.wav", bufferYouWin);
	loadWAVFile("Super Mario Down.wav", bufferCollision);

	loadWAVFile("Treadmill.wav", bufferTreadmill);
	loadWAVFile("Smith.wav", bufferSmith);
	loadWAVFile("Bench Press.wav", bufferBenchPress);
	loadWAVFile("Dumbell Rack.wav", bufferDumbbellRack);
	loadWAVFile("Chin up.wav", bufferChinUp);
	loadWAVFile("Champions.wav", bufferDeadlift);

}
// Function to play the "YOU DIED" sound
// Play the background music
void playBackgroundMusic() {
	alSourcei(sourceBackground, AL_BUFFER, bufferBackground);
	alSourcei(sourceBackground, AL_LOOPING, AL_TRUE); // Loop background music
	alSourcePlay(sourceBackground);
}

// Stop the background music
void stopBackgroundMusic() {
	alSourceStop(sourceBackground);
}

// Play the "You Died" sound
void playYouDiedSound() {
	alSourcei(sourceYouDied, AL_BUFFER, bufferYouDied);
	alSourcePlay(sourceYouDied);
}

// Play the "You Win" sound
void playYouWinSound() {
	alSourcei(sourceYouWin, AL_BUFFER, bufferYouWin);
	alSourcePlay(sourceYouWin);
}

// Play the obstacle collision sound
void playCollisionSound() {
	alSourcei(sourceCollision, AL_BUFFER, bufferCollision);
	alSourcePlay(sourceCollision);
}
void playTreadmillSound() {
	alSourcei(sourceTreadmill, AL_BUFFER, bufferTreadmill);
	alSourcePlay(sourceTreadmill);
}

void playSmithSound() {
	alSourcei(sourceSmith, AL_BUFFER, bufferSmith);
	alSourcePlay(sourceSmith);
}

void playBenchPressSound() {
	alSourcei(sourceBenchPress, AL_BUFFER, bufferBenchPress);
	alSourcePlay(sourceBenchPress);
}

void playDumbbellRackSound() {
	alSourcei(sourceDumbbellRack, AL_BUFFER, bufferDumbbellRack);
	alSourcePlay(sourceDumbbellRack);
}

void playChinUpSound() {
	alSourcei(sourceChinUp, AL_BUFFER, bufferChinUp);
	alSourcePlay(sourceChinUp);
}

void playDeadliftSound() {
	alSourcei(sourceDeadlift, AL_BUFFER, bufferDeadlift);
	alSourcePlay(sourceDeadlift);
}

// Cleanup OpenAL
void cleanupOpenAL() {
	alDeleteSources(1, &sourceBackground);
	alDeleteSources(1, &sourceYouDied);
	alDeleteSources(1, &sourceYouWin);
	alDeleteSources(1, &sourceCollision);

	alDeleteSources(1, &sourceTreadmill);
	alDeleteSources(1, &sourceSmith);
	alDeleteSources(1, &sourceBenchPress);
	alDeleteSources(1, &sourceDumbbellRack);
	alDeleteSources(1, &sourceChinUp);
	alDeleteSources(1, &sourceDeadlift);

	alDeleteBuffers(1, &bufferBackground);
	alDeleteBuffers(1, &bufferYouDied);
	alDeleteBuffers(1, &bufferYouWin);
	alDeleteBuffers(1, &bufferCollision);

	alDeleteBuffers(1, &bufferTreadmill);
	alDeleteBuffers(1, &bufferSmith);
	alDeleteBuffers(1, &bufferBenchPress);
	alDeleteBuffers(1, &bufferDumbbellRack);
	alDeleteBuffers(1, &bufferChinUp);
	alDeleteBuffers(1, &bufferDeadlift);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}


#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

enum GameState { ACTIVE, WIN, LOSE };
GameState gameState = ACTIVE;

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera() {
		setFrontView();
	}

	void setFrontView() {
		eye = Vector3f(2.0f, 2.0f, 9.0f);   // Positioned further back on the Z-axis for front view
		center = Vector3f(2.0f, 2.0f, 0.0f); // Looking towards the center of the room
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void setTopView() {
		eye = Vector3f(2.0f, 7.0f, 1.0f);    // Higher on Y-axis for a top-down view
		center = Vector3f(2.0f, 0.0f, 1.0f); // Looking directly down at the center of the room
		up = Vector3f(0.0f, 0.0f, -1.0f);
	}

	void setSideView() {
		eye = Vector3f(5.0f, 2.0f, 5.0f);    // Position at the edge of the right wall
		center = Vector3f(2.0f, 1.0f, 2.0f); // Looking towards the center from the side
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void setFrontCloseView() {  // Close-up during bending phase
		eye = Vector3f(3.5f, 1.0f, 3.5f);
		center = Vector3f(0.0f, 0.5f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void setSideLiftView() {  // Opposite side view during lifting phase
		eye = Vector3f(0.0f, 0.5f, 1.0f);  // Move eye to the left side
		center = Vector3f(2.0f, 0.5f, 2.0f); // Looking towards the center from the opposite side
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void setOverheadView() {  // Overhead view during holding phase
		eye = Vector3f(1.5f, 3.5f, 1.5f);
		center = Vector3f(0.0f, 1.0f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void look() {
		gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);
	}
};


Camera camera;


struct BoundingBox {
	float minX, maxX;
	float minY, maxY;
	float minZ, maxZ;
};




void drawWall(double thickness, double width, double height) {
	glPushMatrix();
	glScaled(width, thickness, height); // Scale the wall size
	glutSolidCube(1);
	glPopMatrix();
}

void drawTableLeg(double thick, double len) {
	glPushMatrix();
	glTranslated(0, len / 2, 0);
	glScaled(thick, len, thick);
	glutSolidCube(1.0);
	glPopMatrix();
}
void drawJackPart() {
	glPushMatrix();
	glScaled(0.2, 0.2, 1.0);
	glutSolidSphere(1, 15, 15);
	glPopMatrix();
	glPushMatrix();
	glTranslated(0, 0, 1.2);
	glutSolidSphere(0.2, 15, 15);
	glTranslated(0, 0, -2.4);
	glutSolidSphere(0.2, 15, 15);
	glPopMatrix();
}
void drawJack() {
	glPushMatrix();
	drawJackPart();
	glRotated(90.0, 0, 1, 0);
	drawJackPart();
	glRotated(90.0, 1, 0, 0);
	drawJackPart();
	glPopMatrix();
}
void drawTable(double topWid, double topThick, double legThick, double legLen) {
	glPushMatrix();
	glTranslated(0, legLen, 0);
	glScaled(topWid, topThick, topWid);
	glutSolidCube(1.0);
	glPopMatrix();

	double dist = 0.95 * topWid / 2.0 - legThick / 2.0;
	glPushMatrix();
	glTranslated(dist, 0, dist);
	drawTableLeg(legThick, legLen);
	glTranslated(0, 0, -2 * dist);
	drawTableLeg(legThick, legLen);
	glTranslated(-2 * dist, 0, 2 * dist);
	drawTableLeg(legThick, legLen);
	glTranslated(0, 0, -2 * dist);
	drawTableLeg(legThick, legLen);
	glPopMatrix();
}

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}
void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void drawWindowFrame(float width, float height, float depth) {
	glColor3f(0.2f, 0.2f, 0.2f); // Dark gray color for the "TV frame"

	// Draw a solid, filled rectangular block
	glPushMatrix();
	glScaled(width, height, depth);  // Scale to the specified width, height, and depth
	glutSolidCube(1);                // Draw a solid cube scaled to form a rectangular box
	glPopMatrix();
}



float prevPosX = 0.0f, prevPosZ = 0.0f;
float posX = -0.5f, posZ = 1.5f;
float PosY = 0.1f;
float legAngle = 0.0f, armAngle = 0.0f;
float leftarmAngle = -armAngle;



bool isAnimatingBenchPress = false;
float benchPressAnimationTime = 0.0f;
float benchPressDuration = 3.0f;    // Total time for the animation in seconds

float startPosXBench = posX;
float startPosZBench = posZ;
float barLiftAmount = 0.15f;        // Amount the bar moves up and down
float barPosY = 0.6f;               // Starting Y position of the bar
bool isLiftingBar = false;          // Track lifting state for bar



void drawBenchPressSeat() {
	glPushMatrix();
	glColor3f(0.3f, 0.3f, 0.3f);      // Dark gray color for seat
	glTranslated(1.2, 0.3, 0.7);      // Move seat slightly up and outward
	glScaled(0.8, 0.08, 0.25);        // Scale to make seat larger
	glutSolidCube(1);
	glPopMatrix();
}

void drawSeatLeg1() {
	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);      // Dark color for the leg
	glTranslated(0.95, 0.15, 0.7);    // Adjust position of left leg
	glScaled(0.1, 0.3, 0.1);          // Scale to make leg thicker and taller
	glutSolidCube(1);
	glPopMatrix();
}

void drawSeatLeg2() {
	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);      // Dark color for the leg
	glTranslated(1.4, 0.15, 0.7);     // Adjust position of right leg
	glScaled(0.1, 0.3, 0.1);          // Scale to make leg thicker and taller
	glutSolidCube(1);
	glPopMatrix();
}

void drawVerticalSupport1() {
	glPushMatrix();
	glColor3f(0.2f, 0.2f, 0.2f);      // Black color for support
	glTranslated(0.9, 0.5, 1.0);      // Move right side support up and outward
	glScaled(0.1, 0.7, 0.1);          // Scale to make support taller and thicker
	glutSolidCube(1);
	glPopMatrix();
}

void drawVerticalSupport2() {
	glPushMatrix();
	glColor3f(0.2f, 0.2f, 0.2f);      // Black color for support
	glTranslated(0.9, 0.5, 0.4);      // Move left side support up and outward
	glScaled(0.1, 0.7, 0.1);          // Scale to make support taller and thicker
	glutSolidCube(1);
	glPopMatrix();
}

void drawBar() {
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslated(0.9, barPosY, 0.7);  // Use `barPosY` for lifting animation
	glRotated(90, 0.0, 1.0, 0.0);
	glScaled(1.5, 0.08, 0.08);
	glutSolidCube(1);
	glPopMatrix();
}

void drawLeftWeight() {
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f);      // Dark gray color for weights
	glTranslated(0.9, barPosY, 1.25);     // Position left weight further out
	glScaled(0.45, 0.5, 0.1);         // Scale to make weight larger and thicker
	glutSolidCube(1.5);
	glPopMatrix();
}

void drawRightWeight() {
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f);      // Dark gray color for weights
	glTranslated(0.9, barPosY, 0.15);     // Position right weight further out
	glScaled(0.45, 0.5, 0.1);         // Scale to make weight larger and thicker
	glutSolidCube(1.5);
	glPopMatrix();
}





bool isAnimatingSmith = false;      // Track if animatio nis active
float scaleFactor = 1.0f;      // Initial scale factor
float color[3] = { 0.0f, 0.0f, 0.0f }; // Current color (initially black)

// Animation parameters
float targetScale = 1.5f;      // Maximum scale factor
float originalColor[3] = { 0.0f, 0.0f, 0.0f }; // Original color (black)
float maxColor[3] = { 1.0f, 1.0f, 1.0f };      // Color at maximum scale (red)
int animationStep = 0;         // Tracks current stage in animation
int animationSpeed = 80;       // Controls speed of animation steps
int holdDuration = 30;         // Frames to hold at max scale and color





// Draw the base support
void drawBaseSupport() {
	glPushMatrix();
	glColor3f(color[0], color[1], color[2]);      // Use current animation color
	glScaled(1.5 * scaleFactor, 0.1 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the left vertical frame
void drawLeftVerticalFrame() {
	glPushMatrix();
	glColor3f(color[0], color[1], color[2]);
	glTranslated(-0.7 * scaleFactor, 0.5 * scaleFactor, 0); // Apply scale factor to translation
	glScaled(0.1 * scaleFactor, 1.5 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the right vertical frame
void drawRightVerticalFrame() {
	glPushMatrix();
	glColor3f(color[0], color[1], color[2]);
	glTranslated(0.7 * scaleFactor, 0.5 * scaleFactor, 0); // Apply scale factor to translation
	glScaled(0.1 * scaleFactor, 1.5 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the left frame support (diagonal)
void drawLeftFrameSupport() {
	glPushMatrix();
	glColor3f(color[0], color[1], color[2]);
	glTranslated(-0.7 * scaleFactor, 0.25 * scaleFactor, -0.35 * scaleFactor); // Apply scale factor to translation
	glRotated(45, 1.0, 0.0, 0.0);
	glScaled(0.1 * scaleFactor, 1.0 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the right frame support (diagonal)
void drawRightFrameSupport() {
	glPushMatrix();
	glColor3f(color[0], color[1], color[2]);
	glTranslated(0.7 * scaleFactor, 0.25 * scaleFactor, -0.35 * scaleFactor); // Apply scale factor to translation
	glRotated(45, 1.0, 0.0, 0.0);
	glScaled(0.1 * scaleFactor, 1.0 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the bottom support beam (horizontal, connecting left and right frames)
void drawBottomSupport() {
	glPushMatrix();
	glColor3f(color[0], color[1], color[2]);
	glTranslated(0, 0.05 * scaleFactor, -0.55 * scaleFactor); // Apply scale factor to translation
	glScaled(1.3 * scaleFactor, 0.1 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the top horizontal bar
void drawTopBar() {
	glPushMatrix();
	glColor3f(color[0], color[1], color[2]);
	glTranslated(0, 1.2 * scaleFactor, 0); // Apply scale factor to translation
	glScaled(1.5 * scaleFactor, 0.1 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the barbell
void drawBarbell() {
	glPushMatrix();
	glColor3f(0.75f * color[0], 0.75f * color[1], 0.75f * color[2]); // Silver color with scaling effect
	glTranslated(0, 0.8 * scaleFactor, 0); // Apply scale factor to translation
	glScaled(1.9 * scaleFactor, 0.05 * scaleFactor, 0.05 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the left counterweight
void drawLeftCounterweight() {
	glPushMatrix();
	glColor3f(0.3f * color[0], 0.3f * color[1], 0.3f * color[2]); // Darker color with scaling effect
	glTranslated(-0.6 * scaleFactor, 0.75 * scaleFactor, 0); // Apply scale factor to translation
	glScaled(0.1 * scaleFactor, 0.4 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}

// Draw the right counterweight
void drawRightCounterweight() {
	glPushMatrix();
	glColor3f(0.3f * color[0], 0.3f * color[1], 0.3f * color[2]); // Darker color with scaling effect
	glTranslated(0.6 * scaleFactor, 0.75 * scaleFactor, 0); // Apply scale factor to translation
	glScaled(0.1 * scaleFactor, 0.4 * scaleFactor, 0.1 * scaleFactor); // Apply scale factor to scaling
	glutSolidCube(1);
	glPopMatrix();
}



float barHeight = -0.2f;


void drawDeadliftBar() {
	glPushMatrix();
	glColor3f(0.75f, 0.75f, 0.75f);  // Silver color for the bar
	glTranslated(0.0, 0.5, 0.0);     // Position bar above the ground
	glScaled(1.5, 0.05, 0.05);       // Scale to make it a long, thin bar
	glutSolidCube(1);
	glPopMatrix();
}

// Function to draw a dumbbell weight
void drawDumbbell(float radius, float thickness) {
	glColor3f(0.1f, 0.1f, 0.1f);  // Dark color for weights

	// Draw left side of the weight
	glPushMatrix();
	glTranslated(-0.75, 0.5, 0.0);  // Position weight on the left end of the bar
	glScaled(thickness, radius, radius);  // Scale to make a thin, large disc shape
	glutSolidCube(1);  // Draw weigh4
	glPopMatrix();

	// Draw right side of the weight
	glPushMatrix();
	glTranslated(0.75, 0.5, 0.0);  // Position weight on the right end of the bar
	glScaled(thickness, radius, radius);  // Scale to match the left side
	glutSolidCube(1);  // Draw weight
	glPopMatrix();
}

void drawBase() {
	glPushMatrix();
	glColor3f(0.3f, 0.3f, 0.3f);  // Dark gray color for the base
	glScaled(1.2, 0.1, 0.6);      // Scale for the base frame
	glutSolidCube(1);
	glPopMatrix();
}

// Function to draw the running belt
void drawBelt() {
	glPushMatrix();
	glColor3f(0.2f, 0.2f, 0.2f);  // Black color for the running belt
	glTranslated(0.0, 0.05, 0.0); // Position slightly above the base
	glScaled(1.1, 0.02, 0.5);     // Scale for the belt
	glutSolidCube(1);
	glPopMatrix();
}

// Function to draw side rails
void drawSideRails() {
	// Left side rail
	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.6f);  // Light gray color for the side rails
	glRotated(90, 0.0, 1.0, 0.0);
	glTranslated(-0.25, 0.05, 0.2); // Position on the left side
	glScaled(0.1, 0.05, 1.0);     // Scale to make it a long, thin rail
	glutSolidCube(1);
	glPopMatrix();

	// Right side rail
	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.6f);  // Light gray color for the side rails
	glRotated(90, 0.0, 1.0, 0.0);
	glTranslated(0.25, 0.05, 0.2);  // Position on the right side
	glScaled(0.1, 0.05, 1.0);     // Scale to make it a long, thin rail
	glutSolidCube(1);
	glPopMatrix();
}

// Function to draw handles
void drawHandles() {
	// Left handle
	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.6f);  // Light gray for handles
	glTranslated(0.65, 0.4, -0.25); // Position on the left side, above the side rail
	glScaled(0.05, 1.0, 0.05);    // Scale to make it tall and thin
	glutSolidCube(1);
	glPopMatrix();

	// Right handle
	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.6f);  // Light gray for handles
	glTranslated(0.65, 0.4, 0.25);  // Position on the right side, above the side rail
	glScaled(0.05, 1.0, 0.05);    // Scale to make it tall and thin
	glutSolidCube(1);
	glPopMatrix();
}


void drawHandleArms() {
	// Left arm
	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.6f);  // Light gray for arms
	glTranslated(0.5, 0.7, -0.25); // Position on the left side, slightly in front of the handle
	glRotated(90, 0.0, 1.0, 0.0); // Rotate slightly inward
	glScaled(0.05, 0.05, 0.3);     // Scale to make it a short, thin arm
	glutSolidCube(1);
	glPopMatrix();

	// Right arm
	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.6f);  // Light gray for arms
	glTranslated(0.5, 0.7, 0.25);  // Position on the right side, slightly in front of the handle
	glRotated(90, 0.0, 1.0, 0.0);  // Rotate slightly inward
	glScaled(0.05, 0.05, 0.3);     // Scale to make it a short, thin arm
	glutSolidCube(1);
	glPopMatrix();
}
// Function to draw the console
void drawConsole() {
	glPushMatrix();
	glColor3f(0.2f, 0.2f, 0.2f);  // Dark gray for the console
	glTranslated(0.6, 0.8, 0.0);  // Position above the handles
	glRotated(90, 0.0, 1.0, 0.0); // Tilt the console slightly
	glScaled(0.5, 0.2, 0.1);      // Scale for console size
	glutSolidCube(1);
	glPopMatrix();
}
// Draw the horizontal stabilizers (front and back)


// Display function to draw the entire Smith machine

void drawShelf(float width, float depth) {
	glPushMatrix();
	glColor3f(0.5f, 0.5f, 0.5f);  // Light gray color for the shelf
	glScaled(width, 0.05f, depth); // Scale the shelf dimensions
	glutSolidCube(1);
	glPopMatrix();
}

// Function to draw a single dumbbell holder on the shelf
void drawDumbbellHolder(float width, float height, float depth) {
	glPushMatrix();
	glColor3f(0.3f, 0.3f, 0.3f);  // Dark gray color for the holder
	glScaled(width, height, depth); // Scale the holder dimensions
	glutSolidCube(1);
	glPopMatrix();
}

// Function to draw vertical supports for the rack
void drawVerticalSupport(float height) {
	glPushMatrix();
	glColor3f(0.4f, 0.4f, 0.4f);  // Gray color for the vertical supports
	glScaled(0.05f, height, 0.05f); // Scale the support dimensions
	glutSolidCube(1);
	glPopMatrix();
}





// Dumbbell color array to hold changing color values
float dumbbellColor[3] = { 0.1f, 0.1f, 0.1f };


void drawDumbbell() {
	// Dumbbell handle
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);  // Silver color for handle
	glScaled(0.6, 0.07, 0.07);    // Larger handle size
	glutSolidCube(1);
	glPopMatrix();

	// Left weight
	glPushMatrix();
	glColor3f(dumbbellColor[0], dumbbellColor[1], dumbbellColor[2]);  // Use animated color for weights
	glTranslated(-0.35, 0, 0);    // Adjust position for larger weight size
	glScaled(1.3, 1.0, 1.0);      // Scale sphere horizontally for larger weights
	glutSolidSphere(0.1, 20, 20); // Larger spherical weight
	glPopMatrix();

	// Right weight
	glPushMatrix();
	glColor3f(dumbbellColor[0], dumbbellColor[1], dumbbellColor[2]);  // Use animated color for weights
	glTranslated(0.35, 0, 0);     // Adjust position for larger weight size
	glScaled(1.3, 1.0, 1.0);      // Scale sphere horizontally for larger weights
	glutSolidSphere(0.1, 20, 20); // Larger spherical weight
	glPopMatrix();
}

// Function to draw the entire dumbbell rack
void drawDumbbellRack() {
	float shelfWidth = 1.5f;
	float shelfDepth = 0.4f;
	float shelfHeight = 0.3f;

	// Draw bottom shelf
	glPushMatrix();
	glTranslated(0, 0.15f, 0); // Position the bottom shelf close to the ground
	drawShelf(shelfWidth, shelfDepth);
	glPopMatrix();

	// Draw top shelf
	glPushMatrix();
	glTranslated(0, 0.5f, 0); // Position the top shelf higher
	drawShelf(shelfWidth, shelfDepth);
	glPopMatrix();

	// Draw vertical supports
	glPushMatrix();
	glTranslated(-shelfWidth / 2 + 0.05f, 0.35f, -shelfDepth / 2 + 0.05f);
	drawVerticalSupport(0.7f);
	glPopMatrix();

	glPushMatrix();
	glTranslated(shelfWidth / 2 - 0.05f, 0.35f, -shelfDepth / 2 + 0.05f);
	drawVerticalSupport(0.7f);
	glPopMatrix();

	glPushMatrix();
	glTranslated(-shelfWidth / 2 + 0.05f, 0.35f, shelfDepth / 2 - 0.05f);
	drawVerticalSupport(0.7f);
	glPopMatrix();

	glPushMatrix();
	glTranslated(shelfWidth / 2 - 0.05f, 0.35f, shelfDepth / 2 - 0.05f);
	drawVerticalSupport(0.7f);
	glPopMatrix();

	// Draw dumbbell holders on the bottom shelf
	for (int i = -2; i <= 2; i++) {
		glPushMatrix();
		glTranslated(i * 0.3f, 0.18f, 0); // Position holders along the shelf
		drawDumbbellHolder(0.1f, 0.05f, 0.35f);
		glTranslated(0, 0.05, 0.0);
		glRotated(90, 0.0, 1.0, 0.0);
		drawDumbbell();
		glPopMatrix();
	}

	// Draw dumbbell holders on the top shelf
	for (int i = -2; i <= 2; i++) {
		glPushMatrix();
		glTranslated(i * 0.3f, 0.53f, 0); // Position holders along the shelf
		drawDumbbellHolder(0.1f, 0.05f, 0.35f);
		glTranslated(0, 0.05, 0);
		glRotated(90, 0.0, 1.0, 0.0);
		drawDumbbell();
		glPopMatrix();
	}
}

void drawChinUpDipMachine() {
	// Base1
	glPushMatrix();
	glColor3f(0.9f, 0.9f, 0.9f);  // Light gray for the frame
	glScaled(0.1, 0.05, 0.5);    // Scale for the long base
	glTranslated(-1.5, 0, -0.1);     // Position the base
	glutSolidCube(1);
	glPopMatrix();

	// Base2
	glPushMatrix();
	glColor3f(0.9f, 0.9f, 0.9f);  // Light gray for the frame
	glScaled(0.1, 0.05, 0.5);    // Scale for the long base
	glTranslated(1.5, 0, -0.1);     // Position the base
	glutSolidCube(1);
	glPopMatrix();

	// Vertical Supports (left and right)
	glPushMatrix();
	glColor3f(0.9f, 0.9f, 0.9f);  // Light gray for the supports
	glTranslated(-0.15, 0.5, 0);  // Left vertical support position
	glScaled(0.05, 1.0, 0.05);    // Scale for the support height
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.9f, 0.9f, 0.9f);  // Light gray for the supports
	glTranslated(0.15, 0.5, 0);   // Right vertical support position
	glScaled(0.05, 1.0, 0.05);    // Scale for the support height
	glutSolidCube(1);
	glPopMatrix();

	// Horizontal Bar at the Top (for chin-ups)
	glPushMatrix();
	glColor3f(0.9f, 0.9f, 0.9f);  // Light gray for the top bar
	glTranslated(0, 1.0, 0);      // Position the top bar
	glScaled(0.4, 0.05, 0.05);    // Scale for the bar width
	glutSolidCube(1);
	glPopMatrix();

	// Chin-up Handles (angled)
	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);  // Dark color for handles
	glTranslated(-0.18, 1.0, 0.1);  // Left handle position
	glRotated(45, 0, 1, 0);        // Angle the handle
	glScaled(0.15, 0.05, 0.05);    // Scale for the handle
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.1f, 0.1f, 0.1f);  // Dark color for handles
	glTranslated(0.18, 1.0, 0.1);   // Right handle position
	glRotated(-45, 0, 1, 0);       // Angle the handle
	glScaled(0.15, 0.05, 0.05);    // Scale for the handle
	glutSolidCube(1);
	glPopMatrix();


}

bool isWalking = false;
int walkTimer = 0;          // Timer to control the animation duration
const int walkDuration = 50;

float headPosY = 0.3f;
float torsoPosY = 0.0f;
float leftArmPosX = -0.2f, leftArmPosY = 0.0f, leftArmPosZ = 0.0f;
float rightArmPosX = 0.2f, rightArmPosY = 0.0f, rightArmPosZ = 0.0f;
float leftLegPosX = -0.1f, leftLegPosY = -0.3f;
float rightLegPosX = 0.1f, rightLegPosY = -0.3f;


// Rotation angles for animation

float rotationAngle = 0.0f;  // Direction player is facing

float rotationY = 1.0f;
float rotationZ = 0.0f;
float TorsoAngle = 0.0f;

float startPosX = posX;
float startPosZ = posZ;
float rotationBench = 0.0f;


void drawPlayer() {
	glPushMatrix();
	glTranslatef(posX, PosY, posZ);  // Apply general player position for chin-up animation
	glRotatef(rotationAngle, 0.0f, rotationY, rotationZ);  // Face the direction of movement
	glRotatef(rotationBench, 1.0, 0.0, 0.0);

	// Head
	glPushMatrix();
	glColor3f(1.0f, 0.85f, 0.7f);  // Skin tone color
	glTranslated(0.0, headPosY, 0.0); // Use headPosY for chin-up height adjustment

	glutSolidCube(0.2);  // Smaller head cube
	glPopMatrix();

	// Torso
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f); // Black color
	glTranslatef(-0.075f, torsoPosY, 0.0f); // Use torsoPosY for chin-up height adjustment
	glRotatef(TorsoAngle, 0.0f, 1.0f, 0.0f);

	glScalef(0.15f, 0.4f, 0.2f);       // Half the width of the full torso
	glutSolidCube(1.0f);
	glPopMatrix();

	// Right half of the torso (white)
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f); // White color
	glTranslatef(0.075f, torsoPosY, 0.0f); // Use torsoPosY for chin-up height adjustment
	glRotatef(TorsoAngle, 0.0f, 1.0f, 0.0f);

	glScalef(0.15f, 0.4f, 0.2f);       // Half the width of the full torso
	glutSolidCube(1.0f);
	glPopMatrix();

	// Left Arm
	glPushMatrix();
	glColor3f(1.0f, 0.85f, 0.7f);  // Skin tone color
	glTranslated(leftArmPosX, leftArmPosY, leftArmPosZ);  // Position for left arm
	glRotatef(armAngle, 1.0f, 0.0f, 0.0f); // Rotate left arm up
	glScaled(0.15, 0.3, 0.15);     // Reduced scale to make it smaller
	glutSolidCube(1.0);
	glPopMatrix();

	// Right Arm
	glPushMatrix();
	glColor3f(1.0f, 0.85f, 0.7f);  // Skin tone color
	glTranslated(rightArmPosX, rightArmPosY, rightArmPosZ);  // Position for right arm
	glRotatef(leftarmAngle, 1.0f, 0.0f, 0.0f); // Rotate right arm up
	glScaled(0.15, 0.3, 0.15);     // Reduced scale to make it smaller
	glutSolidCube(1.0);
	glPopMatrix();

	// Left Leg
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.8f);   // Dark blue for legs (like pants)
	glTranslated(leftLegPosX, leftLegPosY, 0.0f); // Position for left leg
	glRotatef(legAngle, 1.0f, 0.0f, 0.0f);
	glScaled(0.15, 0.4, 0.15);     // Reduced scale to make it smaller
	glutSolidCube(1.0);
	glPopMatrix();

	// Right Leg
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.8f);   // Dark blue for legs (like pants)
	glTranslated(rightLegPosX, rightLegPosY, 0.0f); // Position for right leg
	glRotatef(-legAngle, 1.0f, 0.0f, 0.0f);
	glScaled(0.15, 0.4, 0.15);     // Reduced scale to make it smaller
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}





// Bounding box for the ChinUpDipMachine based on the dimensions from drawChinUpDipMachine()
BoundingBox getChinUpDipMachineBoundingBox() {
	BoundingBox box;
	box.minX = -3.20f;
	box.maxX = -2.70f;
	box.minY = 0.0f;
	box.maxY = 1.05f;
	box.minZ = -0.35f;
	box.maxZ = 0.35f;
	return box;
}
BoundingBox getBenchPressBoundingBox() {
	BoundingBox box;
	box.minX = -3.20f;
	box.maxX = -2.40f;
	box.minY = 0.0f;
	box.maxY = 1.05f;
	box.minZ = -2.35f;
	box.maxZ = -1.05f;
	return box;
}
BoundingBox getSmithBoundingBox() {
	BoundingBox box;
	box.minX = -1.40f;
	box.maxX = 0.0f;
	box.minY = 0.0f;
	box.maxY = 1.05f;
	box.minZ = -3.2f;
	box.maxZ = -2.8f;
	return box;
}

BoundingBox getTreadMillBoundingBox() {
	BoundingBox box;
	box.minX = 1.0f;
	box.maxX = 3.0f;
	box.minY = 0.0f;
	box.maxY = 1.05f;
	box.minZ = -3.2f;
	box.maxZ = -1.1f;
	return box;
}

BoundingBox getDumbellRackBoundingBox() {
	BoundingBox box;
	box.minX = 1.5f;
	box.maxX = 2.4f;
	box.minY = 0.0f;
	box.maxY = 1.05f;
	box.minZ = -0.3f;
	box.maxZ = 1.4f;
	return box;
}

BoundingBox getDeadLiftBoundingBox() {
	BoundingBox box;
	box.minX = -1.5f;
	box.maxX = 0.5f;
	box.minY = 0.0f;
	box.maxY = 1.05f;
	box.minZ = -0.5f;
	box.maxZ = 0.2f;
	return box;
}

// Bounding box for the player based on their position and size
BoundingBox getPlayerBoundingBox(float posX, float posY, float posZ) {
	BoundingBox box;
	float playerSize = 0.3f; // Adjust based on player size
	box.minX = posX - playerSize / 2;
	box.maxX = posX + playerSize / 2;
	box.minY = posY;
	box.maxY = posY + playerSize; // Adjust for player height
	box.minZ = posZ - playerSize / 2;
	box.maxZ = posZ + playerSize / 2;
	return box;
}

bool checkCollisionChinUp = false;
bool checkCollisionDeadLift = false;
bool checkCollisionBenchPress = false;
bool checkCollisionSmith = false;
bool checkCollisionTreadMill = false;
bool checkCollisionDumbellRack = false;

bool checkCollision(const BoundingBox& box1, const BoundingBox& box2) {
	return (box1.minX <= box2.maxX && box1.maxX >= box2.minX) &&
		(box1.minY <= box2.maxY && box1.maxY >= box2.minY) &&
		(box1.minZ <= box2.maxZ && box1.maxZ >= box2.minZ);
}

BoundingBox chinUpMachineBox = getChinUpDipMachineBoundingBox();
BoundingBox BenchPressBox = getBenchPressBoundingBox();
BoundingBox SmithBox = getSmithBoundingBox();
BoundingBox TreadMillBox = getTreadMillBoundingBox();
BoundingBox DumbellRackBox = getDumbellRackBoundingBox();
BoundingBox DeadLiftBox = getDeadLiftBoundingBox();

void handleSpecialKeyboard(int key, int x, int y) {
	// Reset the timer when a key is pressed
	walkTimer = walkDuration;
	float prevPosX = posX;
	float prevPosZ = posZ;
	// Move player based on key pressed and set rotation angle
	switch (key) {
	case GLUT_KEY_RIGHT:
		if (posX + 0.1f <= 2.2f) {  // Check boundary for the right wall
			posX += 0.1f;
			rotationAngle = 90.0f;
		}
		break;
	case GLUT_KEY_LEFT:
		if (posX - 0.1f >= -3.2f) {  // Check boundary for the left wall
			posX -= 0.1f;
			rotationAngle = -90.0f;
		}
		break;
	case GLUT_KEY_UP:
		if (posZ - 0.1f >= -3.2f) {  // Check boundary for the back wall
			posZ -= 0.1f;
			rotationAngle = 0.0f;
		}
		break;
	case GLUT_KEY_DOWN:
		if (posZ + 0.1f <= 1.9f) {  // Check boundary for the front opening
			posZ += 0.1f;
			rotationAngle = 180.0f;
		}
		break;
	default:
		break;
	}
	startPosX = posX;
	startPosZ = posZ;
	BoundingBox playerBox = getPlayerBoundingBox(posX, 0.0f, posZ);

	if (checkCollision(playerBox, chinUpMachineBox)) {
		// Collision detected; reset position
		posX = prevPosX;
		posZ = prevPosZ;
		checkCollisionChinUp = true;
		walkTimer = 0;
	}
	else {
		checkCollisionChinUp = false;
	}
	if (checkCollision(playerBox, BenchPressBox)) {
		// Collision detected; reset position
		posX = prevPosX;
		posZ = prevPosZ;
		checkCollisionBenchPress = true;
		walkTimer = 0;
	}
	else {
		checkCollisionBenchPress = false;
	}
	if (checkCollision(playerBox, SmithBox)) {
		// Collision detected; reset position
		posX = prevPosX;
		posZ = prevPosZ;
		checkCollisionSmith = true;
		walkTimer = 0;
	}
	else {
		checkCollisionSmith = false;
	}
	if (checkCollision(playerBox, TreadMillBox)) {
		// Collision detected; reset position
		posX = prevPosX;
		posZ = prevPosZ;
		checkCollisionTreadMill = true;
		walkTimer = 0;
	}
	else {
		checkCollisionTreadMill = false;
	}
	if (checkCollision(playerBox, DumbellRackBox)) {
		// Collision detected; reset position
		posX = prevPosX;
		posZ = prevPosZ;
		checkCollisionDumbellRack = true;
		walkTimer = 0;
	}
	else {
		checkCollisionDumbellRack = false;
	}
	if (checkCollision(playerBox, DeadLiftBox)) {
		// Collision detected; reset position
		checkCollisionDeadLift = true;
	}
	else {
		checkCollisionDeadLift = false;
	}
	glutPostRedisplay(); // Redraw the screen
}



bool isAnimatingChinUp = false;
float animationTime = 0.0f;    // Track the elapsed time in seconds
float chinUpDuration = 2.0f;   // Total time for the full up-and-down motion in seconds
float startPosY = 0.1f;        // Starting position of the player
float endPosY = 0.4f;          // Target position for the highest point of the chin-up



void startChinUpAnimation() {
	isAnimatingChinUp = true;
	animationTime = 0.0f;  // Reset elapsed time
	PosY = startPosY;      // Start at the initial position
	armAngle = 135.0f;      // Initial arm angle for gripping position
	rotationAngle = -90;
}

void updateChinUpAnimation(float deltaTime) {
	if (isAnimatingChinUp) {
		animationTime += deltaTime;  // Update the time elapsed

		// Calculate progress as a fraction of the total duration (0.0 to 1.0)
		float progress = fmod(animationTime, chinUpDuration) / chinUpDuration;

		// Use sine to make a smooth up-and-down motion
		float heightFactor = sin(progress * 3.14159f);  // Sine function for smooth up and down
		PosY = startPosY + (endPosY - startPosY) * heightFactor;

		// Arm bending based on position

		// Adjust the Y positions of each part to match the chin-up height
		headPosY = 0.3f + PosY;
		torsoPosY = 0.0f + PosY;
		leftArmPosY = rightArmPosY = 0.35f;
		leftLegPosY = -0.3f + PosY;
		rightLegPosY = -0.3f + PosY;
		posX = -2.8f;
		posZ = 0.0f;
		// Stop the animation after a complete cycle (up and down)
	}
}
void startBenchPressAnimation() {
	isAnimatingBenchPress = true;
	benchPressAnimationTime = 0.0f;
	armAngle = 180;               // Set initial arm position for gripping
	leftarmAngle = 180;
	rotationAngle = 90.0f;

	rotationY = 1.0f;
	rotationZ = 0.0f;

}


void updateBenchPressAnimation(float deltaTime) {
	if (isAnimatingBenchPress) {
		benchPressAnimationTime += deltaTime;

		// Phase 1: Move the player to a seated position on the bench
		if (benchPressAnimationTime < benchPressDuration / 3) {
			rotationBench = -90.0f;
			posX = -2.7f;
			posZ = -1.75f;
			PosY = -0.1f;
		}
		else {
			// Phase 2: Arm adjustment and lift bar up/down
			rightArmPosY = 0.25f;
			leftArmPosY = 0.25f;
			leftArmPosZ = 0.1 + barLiftAmount * sin(benchPressAnimationTime * 3.14f);
			rightArmPosZ = 0.1 + barLiftAmount * sin(benchPressAnimationTime * 3.14f);
			armAngle = 90;
			leftarmAngle = 90;

			// Lift or lower bar with arms
			barPosY = 0.7f + barLiftAmount * sin(benchPressAnimationTime * 3.14f);
		}
	}
}
static int holdCounter = 0;
void updateSmithAnimation(int value) {
	if (!isAnimatingSmith) return;

	switch (animationStep) {
	case 0:  // Scaling up
		scaleFactor += 0.05f;
		for (int i = 0; i < 3; i++) {
			color[i] = originalColor[i] + (maxColor[i] - originalColor[i]) * (scaleFactor - 1) / (targetScale - 1);
		}
		if (scaleFactor >= targetScale) {
			scaleFactor = targetScale;
			animationStep = 1;  // Move to hold phase
		}
		break;

	case 1:  // Hold at maximum size and color
		// Remain in this state indefinitely until 'P' key is pressed
		break;

	case 2:  // Scaling down
		scaleFactor -= 0.05f;
		for (int i = 0; i < 3; i++) {
			color[i] = maxColor[i] - (maxColor[i] - originalColor[i]) * (targetScale - scaleFactor) / (targetScale - 1);
		}
		if (scaleFactor <= 1.0f) {
			scaleFactor = 1.0f;
			animationStep = 3;  // Move to reset phase
		}
		break;

	case 3:  // Reset to original color and stop animation
		for (int i = 0; i < 3; i++) {
			color[i] = originalColor[i];
		}
		isAnimatingSmith = false;
		animationStep = 0;
		break;
	}

	glutPostRedisplay();
	if (isAnimatingSmith) {
		glutTimerFunc(animationSpeed, updateSmithAnimation, 0);
	}
}



bool isAnimatingTreadmill = false;
float treadmillAnimationTime = 0.0f;       // Track elapsed time in seconds
float treadmillRunDuration = 5.0f;        // Total time player runs on treadmill
float legSwingSpeed = 15.0f;               // Speed of swinging motion for legs (higher value = faster motion)
float startLegAngle = 0.0f;                // Initial leg angle
float maxLegAngle = 45.0f;                 // Max leg angle for running motion
float startArmAngle = 0.0f;                // Initial arm angle
float maxArmAngle = 30.0f;                 // Max arm angle for running motion
float startTorsoAngle = 0.0f;
float maxTorsoAngle = 10.0f;

// Start the treadmill animation
void startTreadmillAnimation() {
	isAnimatingTreadmill = true;
	treadmillAnimationTime = 0.0f;  // Reset elapsed time
	legAngle = startLegAngle;       // Start at initial leg angle
	armAngle = startArmAngle;       // Start at initial arm angle
	leftarmAngle = -startArmAngle;  // Opposite angle for left arm
	posX = 1.7f;                    // Set position on treadmill
	PosY = 0.1f;
	posZ = -1.5f;
	rotationAngle = 90.0f;         // Face the treadmill
	TorsoAngle = startTorsoAngle;
}

// Update treadmill animation
void updateTreadmillAnimation(float deltaTime) {
	if (isAnimatingTreadmill) {
		treadmillAnimationTime += deltaTime;  // Update elapsed time

		// Use a sine wave to create back-and-forth swinging motion for legs and arms
		float progress = fmod(treadmillAnimationTime * legSwingSpeed, treadmillRunDuration) / treadmillRunDuration;
		float angleFactor = sin(progress * 3.14159f * 2);

		// Update leg and arm angles based on the angle factor
		legAngle = startLegAngle + maxLegAngle * angleFactor;
		armAngle = startArmAngle + maxArmAngle * -angleFactor;
		leftarmAngle = -(startArmAngle + maxArmAngle * -angleFactor);
		TorsoAngle = startTorsoAngle + maxTorsoAngle * angleFactor;
	}
}

bool isColorChanging = false;  // Flag to toggle color-changing animation
float colorChangeTime = 0.0f;  // Timer to track color change progress


void updateDumbbellColor(float deltaTime) {
	if (isColorChanging) {
		colorChangeTime += deltaTime;  // Increase the timer based on deltaTime

		// Create oscillating RGB values with sine functions for smooth color transitions
		dumbbellColor[0] = 0.1f + 0.5f * sin(colorChangeTime * 2.0f);
		dumbbellColor[1] = 0.1f + 0.5f * sin(colorChangeTime * 2.5f);
		dumbbellColor[2] = 0.1f + 0.5f * sin(colorChangeTime * 3.0f);

	}
	else {
		dumbbellColor[0] = 0.1f;
		dumbbellColor[1] = 0.1f;
		dumbbellColor[2] = 0.1f;
	}
}



float deadliftRotationAngle = 0.0f;
float dumbellRackRotationAngle = 0.0f;

float holdingPhaseCameraAngle = 0.0f; // Angle for rotating camera
const float holdingCameraSpeed = 0.3f; // Adjust speed of rotation as needed


bool isLifting = false;
float deadliftAnimationTime = 0.0f;



// Constants for timing each animation phase
const float bendDownDuration = 5.0f; // seconds
const float liftUpDuration = 5.0f;   // seconds
const float holdUpDuration = 4.0f;     // seconds

void startDeadliftAnimation() {
	isLifting = true;
	deadliftAnimationTime = 0.0f;
	armAngle = leftarmAngle = barHeight = 0.0f;
}

void updateDeadliftAnimation(float deltaTime) {
	if (isLifting) {
		deadliftAnimationTime += deltaTime;
		deadliftRotationAngle = 0.0f;

		if (deadliftAnimationTime <= bendDownDuration) {  // Bending down phase
			alSourceStop(sourceBackground);
			camera.setFrontCloseView();  // Set camera for close-up front view
			float progress = deadliftAnimationTime / bendDownDuration;
			posX = -0.5f;
			posZ = -0.29f;
			rotationAngle = 180;
			armAngle = leftarmAngle = 90.0f * progress;
			barHeight = 0.1f * progress;
		}
		else if (deadliftAnimationTime <= bendDownDuration + liftUpDuration) {  // Lifting up phase
			camera.setSideLiftView();  // Set camera for side view during lift
			float progress = (deadliftAnimationTime - bendDownDuration) / liftUpDuration;
			armAngle = leftarmAngle = 90.0f + 90.0f * progress;
			barHeight = 0.1f + 0.415f * progress;
			leftArmPosY = rightArmPosY = 0.1f + 0.215f * progress;
		}
		else if (deadliftAnimationTime <= bendDownDuration + liftUpDuration + holdUpDuration) {  // Holding phase
			holdingPhaseCameraAngle += holdingCameraSpeed * deltaTime;

			// Calculate the new camera position in a circular path around the player
			float radius = 2.5f; // Radius of the camera circle
			camera.eye.x = radius * cos(holdingPhaseCameraAngle) + 2.0f;  // X position
			camera.eye.z = radius * sin(holdingPhaseCameraAngle) + 2.0f;  // Z position
			camera.eye.y = 3.5f - radius * sin(holdingPhaseCameraAngle);
			barHeight = 0.57f;
		}
		else {
			isLifting = false;  // End animation
			barHeight = -0.2f;
			leftArmPosY = rightArmPosY = 0.0f;
			camera.setFrontView();
			gameState = WIN;
			alSourceStop(sourceDeadlift);
		}
	}
}


bool DeadLiftUsed = false;
bool BenchPressUsed = false;
bool TreadMillUsed = false;
bool ChinUpUsed = false;
bool DumbellRackUsed = false;
bool SmithUsed = false;


void handleKeyboard(unsigned char key, int x, int y) {
	// Check if "E" is pressed and player is colliding with the chin-up machine
	const float moveStep = 0.2f;
	switch (key)
	{
	case 't':
		camera.setTopView();
		break;
	case 'f':
		camera.setFrontView();
		break;
	case 's':
		camera.setSideView();
		break;
	case 'g':  // Move right along X-axis
		camera.eye.x += moveStep;
		camera.center.x += moveStep;
		break;
	case 'h':  // Move left along X-axis
		camera.eye.x -= moveStep;
		camera.center.x -= moveStep;
		break;

		// Move along the Y-axis
	case 'j':  // Move up along Y-axis
		camera.eye.y += moveStep;
		camera.center.y += moveStep;
		break;
	case 'k':  // Move down along Y-axis
		camera.eye.y -= moveStep;
		camera.center.y -= moveStep;
		break;

		// Move along the Z-axis
	case 'l':  // Move forward along Z-axis
		camera.eye.z -= moveStep;
		camera.center.z -= moveStep;
		break;
	case ';':  // Move backward along Z-axis
		camera.eye.z += moveStep;
		camera.center.z += moveStep;
		break;
	case '/':
		deadliftAnimationTime += 0.1;
		break;
	case 27:  // Escape key
		exit(0);
		break;
	default:
		break;
	}
	if (key == 'E' || key == 'e') {  // Check for both uppercase and lowercase
		if (checkCollisionChinUp && !isAnimatingChinUp) {
			startChinUpAnimation();  // Start the chin-up animation
			ChinUpUsed = true;
			playChinUpSound();
		}
		if (checkCollisionBenchPress && !isAnimatingBenchPress) {
			startBenchPressAnimation();  // Start the chin-up animation
			BenchPressUsed = true;
			playBenchPressSound();
		}
		if (checkCollisionSmith && !isAnimatingSmith) {
			isAnimatingSmith = true;
			animationStep = 0;       // Start scaling up
			scaleFactor = 1.0f;      // Reset scale factor
			glutTimerFunc(animationSpeed, updateSmithAnimation, 0);
			SmithUsed = true;
			playSmithSound();
		}
		if (checkCollisionTreadMill && !isAnimatingTreadmill) {
			startTreadmillAnimation();  // Start the chin-up animation
			TreadMillUsed = true;
			playTreadmillSound();
		}
		if (checkCollisionDumbellRack) {
			isColorChanging = true;
			DumbellRackUsed = true;
			playDumbbellRackSound();
		}
		if (checkCollisionDeadLift && !isLifting && DumbellRackUsed && TreadMillUsed && SmithUsed && BenchPressUsed && ChinUpUsed) {
			startDeadliftAnimation();
			playDeadliftSound();
		}
	}
	if (key == 'P' || key == 'p') {  // Check for both uppercase and lowercase
		if (checkCollisionChinUp && isAnimatingChinUp) {
			isAnimatingChinUp = false;  // End the animation after one full cycle
			PosY = 0;           // Reset position
			armAngle = 0.0f;           // Reset arm angle
			leftarmAngle = 0.0f;
			headPosY = 0.3f;
			torsoPosY = 0.0f;
			leftLegPosY = rightLegPosY = -0.3f;
			leftArmPosY = rightArmPosY = 0.0f;
			posX = startPosX;
			posZ = startPosZ;
			rotationAngle = -90;
			animationTime = 0;
			alSourceStop(sourceChinUp);
		}
		if (checkCollisionBenchPress && isAnimatingBenchPress) {
			isAnimatingBenchPress = false;
			posX = startPosX;      // Reset X position
			posZ = startPosZ;      // Reset Z position
			PosY = 0.1f;                // Reset height
			armAngle = 0.0f;            // Reset arms
			leftarmAngle = 0.0f;
			barPosY = 0.6f;             // Reset bar height
			rotationAngle = 0.0f;
			rotationY = 1.0f;

			rotationZ = 0.0f;
			rightArmPosY = 0.0f;
			leftArmPosY = 0.0f;
			rightArmPosZ = 0.0f;
			leftArmPosZ = 0.0f;
			rightArmPosX = -0.2f;
			leftArmPosX = 0.2f;
			rotationBench = 0.0f;
			benchPressAnimationTime = 0.0f;  // Reset animation time
			alSourceStop(sourceBenchPress);
		}
		if (checkCollisionSmith && isAnimatingSmith) {
			animationStep = 2;       // Start scaling down phase
		}
		if (checkCollisionTreadMill && isAnimatingTreadmill) {
			isAnimatingTreadmill = false;
			legAngle = 0.0f;
			armAngle = 0.0f;
			leftarmAngle = 0.0f;
			TorsoAngle = 0.0f;
			// Reset player position if desired
			posX = startPosX;
			posZ = startPosZ;
			PosY = 0.1f;
			alSourceStop(sourceTreadmill);
		}
		if (checkCollisionDumbellRack && isColorChanging) {
			isColorChanging = false;
			colorChangeTime = 0.0f;   // Start the chin-up animation
			alSourceStop(sourceDumbbellRack);
		}
	}

}



float timeRemaining = 90.0f;  // Start timer at 90 seconds
float WallColor[3] = { 0.9f, 0.9f, 0.9f };

std::chrono::time_point<std::chrono::steady_clock> lastUpdate = std::chrono::steady_clock::now();
float colorUpdateInterval = 10.0f;  // Interval in seconds for WallColor adjustment

void updateTimer() {
	// Calculate elapsed time since the last call
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - lastUpdate;

	// Update remaining time
	timeRemaining -= elapsed.count();

	// Check if timeRemaining has reached zero
	if (timeRemaining <= 0.0f) {
		timeRemaining = 0.0f;
		gameState = LOSE;  // Set game state to LOSE when time is up
	}

	// Decrease WallColor every 10 seconds if not already at minimum
	if (static_cast<int>(timeRemaining) % 10 == 0 && timeRemaining <= colorUpdateInterval * 9) {
		for (int i = 0; i < 3; i++) {
			if (WallColor[i] > 0.0f) {
				WallColor[i] -= 0.1f;
				if (WallColor[i] < 0.0f) WallColor[i] = 0.0f;  // Clamp color to minimum 0
			}
		}
		colorUpdateInterval -= 10.0f;  // Update interval to avoid repeated reduction
	}

	// Reset the last update time
	lastUpdate = now;
}



void displayTimer() {
	char timerText[16];
	sprintf(timerText, "Time: %.0f", timeRemaining);  // Format as "Time: X"

	// Display timerText at a specific location on the screen
	glRasterPos3f(3.0f, 4.0f, 3.0f);  // Adjust position as needed
	for (char* c = timerText; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}
}

void displayWinScreen() {
	// Clear the screen with a background color for the win screen
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // Black background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set text color to a bright color
	  // Magenta color for "GAME WIN!"

	// Position and display the "Game Win" message
	glRasterPos2f(1.5f, 2.0f);  // Center the text

	// Draw main text

	const char* winText = "YOU ARE THE WORLD CHAMPION!";
	for (const char* c = winText; *c != '\0'; c++) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);  // Larger font
	}

	// Optional: Add an outline by drawing slightly offset text layers
	glColor3f(0.0f, 1.0f, 1.0f);  // Yellow color for outline effect
	glRasterPos2f(1.51f, 2.01f); // Slightly offset position
	for (const char* c = winText; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}

	// Display some additional celebratory text below

	// Swap buffers to render the screen
	glutSwapBuffers();
}

void displayLoseScreen() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // Black background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set text color to a bright color
	  // Magenta color for "GAME WIN!"

	// Position and display the "Game Win" message
	glRasterPos2f(1.5f, 2.0f);  // Center the text

	// Draw main text

	const char* winText = "YOU WERE TOO WEAK";
	for (const char* c = winText; *c != '\0'; c++) {
		glColor3f(1.0f, 1.0f, 1.0f);
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);  // Larger font
	}

	// Optional: Add an outline by drawing slightly offset text layers
	glColor3f(0.0f, 1.0f, 1.0f);  // Yellow color for outline effect
	glRasterPos2f(1.51f, 2.01f); // Slightly offset position
	for (const char* c = winText; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}
}


void updateAnimation() {
	// If the timer is active, update the arm and leg angles for animation
	if (walkTimer > 0) {
		float time = glutGet(GLUT_ELAPSED_TIME) * 0.01f;
		legAngle = sin(time) * 10.0f;  // Swing legs with sine wave
		armAngle = sin(time) * 20.0f;
		leftarmAngle = -armAngle;
		walkTimer--;                   // Decrease timer
	}
	else {
		// Reset angles when not walking
		legAngle = 0.0f;
		armAngle = 0.0f;
		leftarmAngle = 0.0f;
	}

	deadliftRotationAngle += 0.05f;  // Adjust the value for desired speed
	if (deadliftRotationAngle > 360.0f) {
		deadliftRotationAngle -= 360.0f;  // Keep it within 0-360 degrees
	}
	dumbellRackRotationAngle += 0.05f;  // Adjust the value for desired speed
	if (dumbellRackRotationAngle > 360.0f) {
		dumbellRackRotationAngle -= 360.0f;  // Keep it within 0-360 degrees
	}
}
void idle() {

	updateAnimation();  // Update arm and leg animation angles
	glutPostRedisplay();  // Redisplay for smooth animation
}



bool winSoundPlayed = false;
bool loseSoundPlayed = false;




void Display() {

	if (gameState == WIN) {
		stopBackgroundMusic();


		// Play win sound once
		if (!winSoundPlayed) {
			playYouWinSound();
			winSoundPlayed = true;
		}
		setupCamera();
		setupLights();
		glColor3f(0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glutSwapBuffers();
		camera.setFrontView();

		displayWinScreen();  // Display win screen if game is won
		glFlush();

	}
	else if (gameState == ACTIVE) {

		setupCamera();
		setupLights();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		displayTimer();
		updateTimer();
		updateDeadliftAnimation(0.001);
		updateDumbbellColor(0.005);
		updateTreadmillAnimation(0.001);
		updateBenchPressAnimation(0.001);
		updateChinUpAnimation(0.001);
		glutSwapBuffers();
		//Player
		glPushMatrix();
		glTranslated(2.5, 0.5, 2.0);
		drawPlayer();
		glPopMatrix();
		//Chin up machine
		glPushMatrix();
		glTranslated(-0.5, 0.1, 2.0);
		glRotated(90, 0.0, 1.0, 0.0);
		drawChinUpDipMachine();

		glPopMatrix();
		//Dumbell Rack
		glPushMatrix();
		glRotatef(90, 0.0f, 1.0f, 0.0f);
		glTranslated(-2.5, 0.1, 4.5);

		drawDumbbellRack();
		glPopMatrix();
		//Treadmill 1
		glPushMatrix();
		glTranslated(4.1, 0.1, -1.0);
		drawBase();       // Draw the base of the treadmill
		drawBelt();       // Draw the running belt
		drawSideRails();  // Draw side rails on both sides
		drawHandles();
		drawHandleArms();
		drawConsole();
		glPopMatrix();
		//Treadmill 2
		glPushMatrix();
		glTranslated(4.1, 0.1, -0.2);
		drawBase();       // Draw the base of the treadmill
		drawBelt();       // Draw the running belt
		drawSideRails();  // Draw side rails on both sides
		drawHandles();
		drawHandleArms();
		drawConsole();
		glPopMatrix();
		//Treadmill 3
		glPushMatrix();

		glTranslated(4.1, 0.1, 0.6);
		drawBase();       // Draw the base of the treadmill
		drawBelt();       // Draw the running belt
		drawSideRails();  // Draw side rails on both sides
		drawHandles();
		drawHandleArms();
		drawConsole();
		glPopMatrix();

		//Deadlift Bar
		glPushMatrix();
		glTranslated(2.0, barHeight, 1.8);  // Position the bar
		glRotatef(deadliftRotationAngle, 0.0f, 1.0f, 0.0f);  // Rotate around the Y-axis
		drawDeadliftBar();
		drawDumbbell(0.4, 0.4);
		glPopMatrix();

		//Bench press
		glPushMatrix();
		glTranslated(-1.3, 0.0, -0.4);  // Center in the larger room
		drawBenchPressSeat();
		drawSeatLeg1();
		drawSeatLeg2();
		drawVerticalSupport1();
		drawVerticalSupport2();
		drawBar();
		drawLeftWeight();
		drawRightWeight();
		glPopMatrix();

		//Smith Machine
		glPushMatrix();
		glTranslated(1.8, 0.0, -0.7);
		drawBaseSupport();
		drawLeftVerticalFrame();
		drawRightVerticalFrame();
		drawTopBar();
		drawBarbell();
		drawLeftCounterweight();
		drawRightCounterweight();
		drawLeftFrameSupport();
		drawRightFrameSupport();
		drawBottomSupport();
		glPopMatrix();

		// Ground wall (floor) - light brown
		glPushMatrix();
		glColor3f(0.76f, 0.6f, 0.42f); // Light brown color
		glTranslated(2.0, 0.0, 1.0);    // Centered on ground level
		drawWall(0.02, 6.0, 6.0);       // Increased width significantly
		glPopMatrix();

		// Left wall - light gray with window frame
		glPushMatrix();
		glColor3f(WallColor[0], WallColor[1], WallColor[2]); // Light gray color
		glTranslated(-1.0, 2.0, 1.0);   // Move to the left side
		glRotated(90, 0, 0, 1.0);
		drawWall(0.02, 4.0, 6.0);       // Adjusted height to match back wall
		glTranslated(0.0, -0.2, 0.5);    // Slightly offset frame outward
		glRotated(-90, 1.0, 0, 0);
		drawWindowFrame(1.5, 1.0, 0.05); // Window frame with width, height, thickness

		glPopMatrix();

		// Back wall - light gray with window frame
		glPushMatrix();


		glTranslated(2.0, 2.0, -1.5);   // Centered back, made wider
		drawWindowFrame(4.0, 2.0, 0.05); // Window frame with width, height, thickness
		glRotated(-90, 1.0, 0.0, 0.0);
		glColor3f(WallColor[0], WallColor[1], WallColor[2]); // Light gray color
		drawWall(0.02, 6.0, 4.0);       // Increased width significantly
		glPopMatrix();

		// Right wall - light gray with window frame
		glPushMatrix();

		glTranslated(5.0, 2.0, 1.0);    // Move to the right side

		glRotated(90, 0, 0, 1.0);

		glColor3f(WallColor[0], WallColor[1], WallColor[2]); // Light gray color
		drawWall(0.02, 4.0, 6.0);       // Adjusted height to match back wall
		glTranslated(0.0, 0.0, 0.5);    // Slightly offset frame outward
		glRotated(-90, 1.0, 0, 0);
		drawWindowFrame(1.5, 1.0, 0.05); // Window frame with width, height, thickness

		glPopMatrix();

		glFlush();
	}
	else {
		stopBackgroundMusic();


		// Play lose sound once
		if (!loseSoundPlayed) {
			playYouDiedSound();
			loseSoundPlayed = true;
		}
		setupCamera();
		setupLights();
		glColor3f(0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glutSwapBuffers();
		camera.setFrontView();
		displayLoseScreen();  // Display win screen if game is won

		glFlush();

	}
}










void main(int argc, char** argv) {
	glutInit(&argc, argv);
	initOpenAL();
	std::thread soundThread(loadSoundInBackground);
	soundThread.join();
	if (gameState == WIN)
		playYouWinSound();
	else if (gameState == ACTIVE)
		playBackgroundMusic();
	else
		playYouDiedSound();
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("Roblox el 8alaba");
	glutDisplayFunc(Display);
	glutIdleFunc(idle);
	glutSpecialFunc(handleSpecialKeyboard);
	glutKeyboardFunc(handleKeyboard);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();

	cleanupOpenAL();
}