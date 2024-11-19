//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Adam Zsombor
// Neptun : X079FB
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char* vertexSource = R"(
    #version 330
    precision highp float;

    uniform mat4 MVP;

    layout(location = 0) in vec2 vertexPosition;

    void main() {
        gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, 1) * MVP;
    }
)";

const char* fragmentSource = R"(
    #version 330
    precision highp float;

    uniform vec3 color;
    out vec4 fragmentColor;

    void main() {
        fragmentColor = vec4(color, 1);
    }
)";

class Camera {
public:
    
    float centerX, centerY, worldWidth, worldHeight;
    
    Camera() {
        centerX = 0;
        centerY = 0;
        worldWidth = 30;
        worldHeight = 30;
    }

    mat4 view() {
        return TranslateMatrix(vec3 (-centerX, -centerY, 0));
    }

    mat4 projection() {
        return ScaleMatrix(vec3 (2 / worldWidth, 2 / worldHeight, 1));
    }

    mat4 inverseView() {
        return TranslateMatrix(vec3 (centerX, centerY, 0));
    }

    mat4 inverseProjection() {
        return ScaleMatrix(vec3 (worldWidth / 2, worldHeight / 2, 1));
    }
    
    void zoom(float z) {
        worldWidth *= z;
        worldHeight *= z;
    }
    
    void pan(int p) {
        centerX += p;
    }
    
};


Camera camera;
GPUProgram gpuProgram;
const int numberOfVertices = 100;
int picked = -1;
float tenzio = 0;

class Curve {
public:
    
    unsigned int vaoCurve, vboCurve, vaoPoints, vboPoints;
    std::vector<vec4> points;
    
    void buildCurve() {
        glGenVertexArrays(1, &vaoCurve);
        glBindVertexArray(vaoCurve);

        glGenBuffers(1, &vboCurve);
        glBindBuffer(GL_ARRAY_BUFFER, vboCurve);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
    }
    
    void buildPoints() {
        glGenVertexArrays(1, &vaoPoints);
        glBindVertexArray(vaoPoints);

        glGenBuffers(1, &vboPoints);
        glBindBuffer(GL_ARRAY_BUFFER, vboPoints);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    }

    Curve() {
        buildCurve();
        buildPoints();
    }

    virtual vec4 r(float t) {
        return points[0];
    }
    
    virtual float tStart() {
        return 0;
    }
    
    virtual float tEnd() {
        return 1;
    }
    
    vec4 calculateInverse(float x, float y) {
        return vec4(x, y, 0, 1) * camera.inverseProjection() * camera.inverseView();
    }

    virtual void addPoint(float x, float y) {
        points.push_back(calculateInverse(x, y));
    }

    int pickPoint(float x, float y) {
        for (unsigned int p = 0; p < points.size(); p++) {
            if (dot(points[p] - calculateInverse(x, y), points[p] - calculateInverse(x, y)) < 0.1) {
                return p;
            }
        }
        return -1;
    }

    void movePoint(int p, float x, float y) {
        points[p] = calculateInverse(x, y);
    }
    
    void drawCurve(int colorId) {
        std::vector<float> data;
        for (int i = 0; i < numberOfVertices; i++) {
            float t = tStart() + (tEnd() - tStart()) * (float)i / (numberOfVertices - 1);
            vec4 vertex = r(t);
            data.push_back(vertex.x);
            data.push_back(vertex.y);
        }
        glBindVertexArray(vaoCurve);
        glBindBuffer(GL_ARRAY_BUFFER, vboCurve);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_DYNAMIC_DRAW);
        if (colorId > -1) {
            glUniform3f(colorId, 1, 1, 0);
        }
        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_STRIP, 0, numberOfVertices);
    }
    
    void drawPoints(int colorId) {
        glBindVertexArray(vaoPoints);
        glBindBuffer(GL_ARRAY_BUFFER, vboPoints);
        glBufferData(GL_ARRAY_BUFFER, points.size() * 4 * sizeof(float), &points[0], GL_DYNAMIC_DRAW);
        if (colorId > -1) {
            glUniform3f(colorId, 1, 0, 0);
        }
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, (int)points.size());
    }

    void draw() {
        mat4 VPTransform = camera.view() * camera.projection();
        gpuProgram.setUniform(VPTransform, "MVP");
        int colorId = glGetUniformLocation(gpuProgram.getId(), "color");

        if (points.size() > 1) {
            drawCurve(colorId);
        }
        
        if (points.size() >= 1) {
            drawPoints(colorId);
        }

    }
    
    void rmCurve() {
        glDeleteVertexArrays(1, &vaoCurve);
        glDeleteBuffers(1, &vboCurve);
    }
    
    void rmPoints() {
        glDeleteVertexArrays(1, &vaoPoints);
        glDeleteBuffers(1, &vboPoints);
    }
    
    virtual ~Curve() {
        rmCurve();
        rmPoints();
    }
};

class Bezier : public Curve {
public:
    
    float bernstein(int i, float t) {
        float which = 1;
        for (int j = 1; j <= i; j++) {
            which *= (float)(points.size() - j) / j;
        }
        return which * pow(t, i) * pow(1 - t, points.size() - 1 - i);
    }
    
    vec4 r(float t) {
        vec4 point = vec4(0, 0, 0, 0);
        for (int i = 0; i < (int)points.size(); i++) {
            point += points[i] * bernstein(i, t);
        }
        return point;
    }
    
};

class Lagrange : public Curve {
public:
    
    std::vector<float> knots;
    float totalDst = 0;
    
    float L(int i, float t) {
        float Li = 1;
        for (int j = 0; j < (int)points.size(); j++) {
            if (j != i) {
                Li *= (t - knots[j]) / (knots[i] - knots[j]);
            }
        }
        return Li;
    }
    
    float distance(vec4 p1, vec4 p2) {
        return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }
    
    void calculateNewKnotValue(vec4 p1, vec4 p2, int i) {
        knots.push_back(((totalDst * knots[i - 1]) + distance(p1, p2)) / totalDst);
    }
    
    void recalculateKnots() {
        vec4 p1 = points[points.size() - 1];
        vec4 p2 = points[points.size() - 2];
        totalDst += distance(p1, p2);
        knots.clear();
        knots.push_back(0);
        for (int i = 1; i < (int)points.size(); i++) {
            calculateNewKnotValue(points[i - 1], points[i], i);
        }
    }
    
    void addPoint(float x, float y) {
        Curve::addPoint(x, y);
        if (points.size() > 1) {
            recalculateKnots();
        }
    }
    
    float tStart() {
        return knots[0];
    }
    
    float tEnd() {
        return knots[points.size() - 1];
    }

    vec4 r(float t) {
        vec4 point = vec4(0, 0, 0, 0);
        for (int i = 0; i < (int)points.size(); i++) {
            point += points[i] * L(i, t);
        }
        return point;
    }
    
};

class CatmullRom : public Curve {
public:
    
    std::vector<float> knots;
    float totalDst = 0;
    
    vec4 hermite(vec4 p0, vec4 v0, float t0, vec4 p1, vec4 v1, float t1, float t) {
        float dt = t1 - t0;
        vec4 a0 = p0;
        vec4 a1 = v0;
        vec4 a2 = (p1 - p0) * 3 / (dt * dt) - (v1 + v0 * 2) / dt;
        vec4 a3 = (p0 - p1) * 2 / (dt * dt * dt) + (v1 + v0) / (dt * dt);
        t -= t0;
        return ((a3 * t + a2) * t + a1) * t + a0;
    }
    
    float distance(vec4 p1, vec4 p2) {
        return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
    }
    
    void calculateNewKnotValue(vec4 p1, vec4 p2, int i) {
        knots.push_back(((totalDst * knots[i - 1]) + distance(p1, p2)) / totalDst);
    }
    
    void recalculateKnots() {
        vec4 p1 = points[points.size() - 1];
        vec4 p2 = points[points.size() - 2];
        totalDst += distance(p1, p2);
        knots.clear();
        knots.push_back(0);
        for (int i = 1; i < (int)points.size(); i++) {
            calculateNewKnotValue(points[i - 1], points[i], i);
        }
    }
    
    void addPoint(float x, float y) {
        Curve::addPoint(x, y);
        if (points.size() > 1) {
            recalculateKnots();
        }
    }
    
    float tStart() {
        return knots[0];
    }
    
    float tEnd() {
        return knots[points.size() - 1];
    }
    
    vec4 getPrevious(int i) {
        if (i >= 1) {
            return (points[i] - points[i - 1]) / (knots[i] - knots[i - 1]);
        }
        return vec4(0, 0, 0, 0);
    }
    
    vec4 getCurrent(int i) {
        return (points[i + 1] - points[i]) / (knots[i + 1] - knots[i]);
    }
    
    vec4 getNext(int i) {
        if (i < (int)points.size() - 2) {
            return (points[i + 2] - points[i + 1]) / (knots[i + 2] - knots[i + 1]);
        }
        return vec4(0, 0, 0, 0);
    }
    
    vec4 getVector(vec4 p1, vec4 p2) {
        return (p1 + p2) * ((1.0f - tenzio) / 2.0f);
    }
    
    vec4 r(float t) {
        for (int i = 0; i < (int)points.size() - 1; i++) {
            if (knots[i] <= t && knots[i + 1] >= t) {
                
                vec4 previous = getPrevious(i);
                vec4 current = getCurrent(i);
                vec4 next = getNext(i);
                
                return hermite(points[i], getVector(previous, current), knots[i], points[i + 1], getVector(next, current), knots[i + 1], t);
            }
        }
        return points[0];
    }
};

Curve* curve;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    curve = new Curve();
    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

void onDisplay() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    curve->draw();
    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
    
    if (key == 'Z') {
        camera.zoom(1.1f);
    } else if (key == 'z') {
        camera.zoom(1.0f / 1.1f);
    } else if (key == 'P') {
        camera.pan(1);
    } else if (key == 'p') {
        camera.pan(-1);
    } else if (key == 'b') {
        delete curve;
        curve = new Bezier();
    } else if (key == 'l') {
        delete curve;
        curve = new Lagrange();
    } else if (key == 'c') {
        delete curve;
        curve = new CatmullRom();
    } else if (key == 'T') {
        tenzio += 0.1f;
    } else if (key == 't') {
        tenzio -= 0.1f;
    }
    
    glutPostRedisplay();
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouse(int button, int state, int pX, int pY) {
    float cX = 2.0f * pX / windowWidth - 1;
    float cY = 1.0f - 2.0f * pY / windowHeight;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        curve->addPoint(cX, cY);
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        picked = curve->pickPoint(cX, cY);
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
        picked = -1;
    }
    
    glutPostRedisplay();
}

void onMouseMotion(int pX, int pY) {
    float cX = 2.0f * pX / windowWidth - 1;
    float cY = 1.0f - 2.0f * pY / windowHeight;
    if (picked >= 0) {
        curve->movePoint(picked, cX, cY);
    }
}

void onIdle() {
    glutPostRedisplay();
}
