#include <iostream>
//#include "DrawInterface.h"
#include "InputManager.h"
#include "rlgl.h"
#include "raymath.h"
#include <stack>
//#include <Eigen/Dense>

using namespace std;
using namespace ReyEngine;



// Let's also verify the Vector3Transform implementation:
Vector3 myVector3Transform(Vector3 v, Matrix mat) {
   Vector3 result = { 0 };

   float x = v.x;
   float y = v.y;
   float z = v.z;

   result.x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12;    // First row
   result.y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13;    // Second row
   result.z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14;   // Third row

   return result;
}


// And let's verify our MatrixToFloatV implementation:
auto MatrixToFloatV(const Matrix& m) {
   float16 result;
   // Matrix is stored in column-major order
   result.v[0] = m.m0;   result.v[4] = m.m4;   result.v[8] = m.m8;    result.v[12] = m.m12;
   result.v[1] = m.m1;   result.v[5] = m.m5;   result.v[9] = m.m9;    result.v[13] = m.m13;
   result.v[2] = m.m2;   result.v[6] = m.m6;   result.v[10] = m.m10;  result.v[14] = m.m14;
   result.v[3] = m.m3;   result.v[7] = m.m7;   result.v[11] = m.m11;  result.v[15] = m.m15;
   return result;
}


int main(int argc, char** argv){
   Camera2D foregroundCamera = {0}; //BEGIN to draw background
   foregroundCamera.zoom = 1.0;
   Size<int> windowSize(800, 600);
   InitWindow(windowSize.x, windowSize.y, "basic drawsing test");
   SetTargetFPS(60);


   // Define the camera to look into our 3d world
   Camera3D camera = { 0 };
   camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
   camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
   camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
   camera.fovy = 45.0f;                                // Camera field-of-view Y
   camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

   Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
   RenderTexture2D canvasTarget = LoadRenderTexture(windowSize.x, windowSize.y);
   RenderTexture2D scrollTarget = LoadRenderTexture(300, 300);

   auto suzanne = LoadModel("test/suzanne.obj");

   // And let's modify how we print the matrix to verify its structure:
   auto printMat = [](const Matrix& m) {
      std::stringstream ss;
      ss << "Matrix:\n";
      ss << m.m0 << ", " << m.m4 << ", " << m.m8 << ", " << m.m12 << "\n";
      ss << m.m1 << ", " << m.m5 << ", " << m.m9 << ", " << m.m13 << "\n";
      ss << m.m2 << ", " << m.m6 << ", " << m.m10 << ", " << m.m14 << "\n";
      ss << m.m3 << ", " << m.m7 << ", " << m.m11 << ", " << m.m15;
      return ss.str();
   };

   while(!WindowShouldClose()) {
      int moveSpeed = 5;
      Vec2<int> mvVec;
//      double rotation = 0;
//      if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_W)) {
//         mvVec += {0, -1};
//      }
//      if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_A)) {
//         mvVec += {-1, 0};
//      }
//      if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_S)) {
//         mvVec += {0, 1};
//      }
//      if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_D)) {
//         mvVec += {1, 0};
//      }
//      if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_Q)) {
//         rotation += 1;
//      }
//      if (InputManager::isKeyDown(InputInterface::KeyCode::KEY_E)) {
//         rotation -= 1;
//      }
//      if (InputManager::isMouseButtonDown(InputInterface::MouseButton::LEFT)) {
//         //see what the mouse delta is
//         auto delta = InputManager::getMouseDelta();
//         // Define the camera's initial position
////         Eigen::Vector3d cameraPosition(camera.position.x, camera.position.y, camera.position.z);
////
////         // Define the angle of rotation in degrees and convert it to radians
////         double angleDegrees = delta.x;
////         double angleRadians = angleDegrees * M_PI / 180.0;
////
////         // Define the rotation matrix around the y-axis
////         Eigen::Matrix3d rotationMatrix;
////         rotationMatrix = Eigen::AngleAxisd(angleRadians, Eigen::Vector3d::UnitY());
////
////         // Apply the rotation to the camera position
////         Eigen::Vector3d rotatedPosition = rotationMatrix * cameraPosition;
////
////         // Output the result
////         std::cout << "Original position: " << cameraPosition.transpose() << std::endl;
////         std::cout << "Rotated position: " << rotatedPosition.transpose() << std::endl;
////         camera.position.x = rotatedPosition.x();
////         camera.position.y = rotatedPosition.y();
////         camera.position.z = rotatedPosition.z();
//      }
//
//      if (mvVec) {
//         auto newVec = mvVec * moveSpeed;
//         foregroundCamera.offset += newVec;
//         cout << Vec2<float>(foregroundCamera.offset) << endl;
//      }
//      if (rotation) {
//         foregroundCamera.rotation += rotation;
//      }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      2d camera
//      BeginDrawing();
//      ClearBackground(WHITE);
//      // Draw somebackground
//      BeginMode2D(foregroundCamera);
//      {
//         DrawText("CameraBackground", 100, 100, 20, RED);
//         DrawRectangle(100, 200, 20, 20, RED);
//      }
//      EndMode2D();
//
//
//      DrawText("Foreground", 300, 300, 20, BLUE);
//      DrawRectangle(0, 0, 20, 20, BLUE);
//
//      //Draw more background
//      BeginMode2D(foregroundCamera);
//      {
//         DrawText("More background", 500, 300, 20, RED);
//         DrawRectangle(40, 20, 20, 20, RED);
//      }
//      EndMode2D();
//      EndDrawing();

      // Draw somebackground
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Rendering3D to a (nested) texture
//      BeginTextureMode(canvasTarget);              // Begin drawing to render texture
//      {
//         ClearBackground(WHITE);
//         DrawText("This is the Camera UI", 0, windowSize.y - 20, 20, BLACK);
//
//         {
//            BeginMode2D(foregroundCamera);
//            DrawText("This is the 2D background", 400, 300, 20, BLACK);
//
//            {
//               BeginMode3D(camera);
//               DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
//               DrawSphere({1, 1, 1}, 0.5, BLUE);
//               DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
//               DrawGrid(10, 1.0f);
//               DrawModel(suzanne, {5,0,0}, 1, GREEN);
//               EndMode3D();
//            }
//
//            EndMode2D();
//         }
//         EndTextureMode();
//      }
//      //window
//      BeginDrawing();
//      ClearBackground(WHITE);
//      //DRaw the texture
//      DrawTextureRec(canvasTarget.texture, {0, 0, (float)windowSize.x, -(float)windowSize.y}, {0, 0}, WHITE);
//      EndDrawing();
      //end window

      auto getPointInFrame = [](Vector3 point, const Matrix& frameMatrix) {
         return Vector3Transform(point, MatrixInvert(frameMatrix));
      };

      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      2d rotations
//      BeginTextureMode(canvasTarget);              // Begin drawing to render texture
//      {
//         ClearBackground(WHITE);
//         DrawText("This is the Camera UI", 0, windowSize.y - 20, 20, BLACK);
//
//         {
//            BeginMode2D(foregroundCamera);
//            rlPushMatrix();
//               rlTranslatef(100,100,0);
//               rlRotatef(20, 0.0f, 0.0f, 1.0f);        // Rotate around Z axis
//               auto frame1 = rlGetMatrixTransform();
//               cout << printMat(frame1) << endl;
//               DrawText("RotatedOnce!", 0, 0, 20, BLACK);
//               auto origin = getPointInFrame({0,0,0}, frame1);
//               DrawLine(origin.x, origin.y, 50,0, Colors::red);
//
//                  rlTranslatef(100,100,0);
//                  rlRotatef(20, 0.0f, 0.0f, 1.0f);
//                  auto frame2 = rlGetMatrixTransform();
//                  cout << printMat(frame2) << endl;
//                  DrawText("RotatedTwice!", 0, 0, 20, BLACK);
//                  origin = Vector3Transform(getPointInFrame({0,0,0}, frame2), frame1);
//                  DrawLine(origin.x, origin.y, 50,0, Colors::red);
//
//                     rlTranslatef(100,100,0);
//                     rlRotatef(20, 0.0f, 0.0f, 1.0f);
//                     auto frame3 = rlGetMatrixTransform();
//                     cout << printMat(frame3) << endl;
//                     DrawText("RotatedNotOnceNotTwiceButThrice!", 0, 0, 20, BLACK);
//                     origin = (Vector3Transform(getPointInFrame({0,0,0}, frame3), frame2));
//                     DrawLine(origin.x, origin.y, 50,0, Colors::red);
//
//
//            rlPopMatrix();
//            EndMode2D();
//         }
//         EndTextureMode();
//      }
//      //window
//      BeginDrawing();
//      ClearBackground(WHITE);
//      //DRaw the texture
//      rlPushMatrix();
//      rlScalef(2,2,0);
//      DrawTextureRec(canvasTarget.texture, {0, 0, (float)windowSize.x, -(float)windowSize.y}, {0, 0}, WHITE);
//      rlPopMatrix();
//      EndDrawing();
//
//   }

      //scroll area
//      std::stack<Matrix> myStack;
//      BeginTextureMode(canvasTarget);              // Begin drawing to render texture
//      {
//         ClearBackground(WHITE);
//         DrawText("This is the Camera UI", 0, windowSize.y - 20, 20, BLACK);
//
//         {
////            BeginMode2D(foregroundCamera);
//            rlPushMatrix();
//               rlTranslatef(100,100,0);
//               rlRotatef(20, 0.0f, 0.0f, 1.0f);        // Rotate around Z axis
//               auto frame1 = rlGetMatrixTransform();
//               cout << "frame 1 = " << printMat(frame1) << endl;
//               DrawText("RotatedOnce!", 0, 0, 20, BLACK);
//               auto origin = getPointInFrame({0,0,0}, frame1);
//               DrawLine(origin.x, origin.y, 50,0, Colors::red);
//
//                  rlPushMatrix();
//                  rlTranslatef(100,100,0);
//                  rlRotatef(20, 0.0f, 0.0f, 1.0f);
//                  auto frame2 = rlGetMatrixTransform();
//                  cout << printMat(frame2) << endl;
//                  DrawText("RotatedTwice!", 0, 0, 20, BLACK);
//                  origin = Vector3Transform(getPointInFrame({0,0,0}, frame2), frame1);
//                  DrawLine(origin.x, origin.y, 50,0, Colors::red);
//
//                  auto mat = rlGetMatrixTransform();
//                  cout << "Global transform = " << printMat(mat) << endl;
//                  EndTextureMode();
//                     BeginTextureMode(scrollTarget);
//                     rlPushMatrix();
//                        DrawRectangleGradientEx({0,0,(float)scrollTarget.texture.width, (float)scrollTarget.texture.height}, BLUE, RED, GREEN, PURPLE);
//                     rlPopMatrix();
//                     EndTextureMode();
//                  BeginTextureMode(canvasTarget);
//                  rlPushMatrix();
//                     rlMultMatrixf(&mat.m0);
//                     cout << "Restored matrix = " << printMat(rlGetMatrixTransform()) << endl;
//                     DrawTextureRec(scrollTarget.texture, {0, 0, (float)scrollTarget.texture.width, -(float)scrollTarget.texture.height}, {0, 0}, LIGHTGRAY);
//                     DrawText("I am a scroll area!", 0, 0, 20, BLACK);
//                  rlPopMatrix();
//
//            rlPopMatrix();
//            rlPopMatrix();
////            EndMode2D();
//         }
//      }
//      EndTextureMode();
//      //window
//      BeginDrawing();
//      DrawTextureRec(canvasTarget.texture, {0, 0, (float)windowSize.x, -(float)windowSize.y}, {0, 0}, WHITE);
//      EndDrawing();
//
//   }
      static float tick = 0;
      tick++;
      BeginTextureMode(scrollTarget);
      ClearBackground(WHITE);
      rlPushMatrix();
         rlTranslatef(100,100,0);
         DrawText("ScrollArea", 0, 0, 20, BLACK);
         auto deg = 360 * cos(tick)/1;
         rlRotatef(deg, 0,0,1);
         cout << deg << endl;
         DrawRectangleGradientEx({0,0,30, 30}, BLUE, RED, GREEN, PURPLE);
      rlPopMatrix();
      EndTextureMode();

      BeginTextureMode(canvasTarget);              // Begin drawing to render texture
      {
         ClearBackground(WHITE);
         DrawText("hello", 0, windowSize.y - 20, 20, BLACK);
         rlPushMatrix();
            rlTranslatef(100,100,0);
            rlRotatef(45, 0,0,1);
            DrawText("rotated", 0, 0, 20, BLACK);

            rlPushMatrix();
               DrawTextureRec(scrollTarget.texture, {0, 0, (float)scrollTarget.texture.width, -(float)scrollTarget.texture.height}, {0, 0}, WHITE);
            rlPopMatrix();

         rlPopMatrix();
      }
      EndTextureMode();
      //window
      BeginDrawing();
      DrawTextureRec(canvasTarget.texture, {0, 0, (float)windowSize.x, -(float)windowSize.y}, {0, 0}, WHITE);
      EndDrawing();

   }
   return 0;
}