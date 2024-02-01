#include <algorithm>
#include <complex>
#include <raylib.h>
#include <string>
#include <vector>

struct dvec2 {
	double x, y;
};

void InitTexture(RenderTexture2D* texture) {
	if (texture != nullptr) {
		UnloadRenderTexture(*texture);
	}
	*texture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
}

void ScreenToWorld(double* x, double* y, double zoom, dvec2 offset) {
	const dvec2 screenSize = {
		static_cast<double>(GetScreenWidth()),
		static_cast<double>(GetScreenHeight())
	};
	double ndcX = screenSize.x / std::min(screenSize.x, screenSize.y) * 2.0 - 1.0;
	double ndcY = screenSize.y / std::min(screenSize.x, screenSize.y) * 2.0 - 1.0;
	const dvec2 centerOffset = {
		1.0 - ndcX,
		1.0 - ndcY,
	};
	ndcX = *x / std::min(screenSize.x, screenSize.y) * 2.0f - 1.0f;
	ndcY = *y / std::min(screenSize.x, screenSize.y) * 2.0f - 1.0f;

	*x = ndcX / zoom + offset.x + centerOffset.x;
	*y = ndcY / zoom + offset.y + centerOffset.y;
}

bool InputBoxFloat(std::string& buffer, Vector2 pos, Vector2 size, int fontSize, double* value) {
	bool retVal = false;
	const Rectangle box = {pos.x, pos.y, size.x, size.y};
	// input
	if (CheckCollisionPointRec(GetMousePosition(), box)) {
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) buffer.clear();
		SetMouseCursor(MOUSE_CURSOR_IBEAM);
		int key = GetCharPressed();
		while (key > 0) {
			if (key == '-' || key == '.' || ((key >= '0') && (key <= '9'))) {
				buffer.push_back(static_cast<char>(key));
			}
			key = GetCharPressed();
		}
		if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !buffer.empty()) {
			buffer.pop_back();
		}
	}
	else {
		SetMouseCursor(MOUSE_CURSOR_DEFAULT);
		buffer = std::to_string(*value);
	}

	if (IsKeyPressed(KEY_ENTER)) {
		try {
			const double temp = std::stod(buffer);
			retVal = true;
			*value = temp;
		}
		catch (const std::exception&) {
			retVal = false;
		}
	}

	// draw
	DrawRectangleRec(box, LIGHTGRAY);
	if (CheckCollisionPointRec(GetMousePosition(), box)) {
		DrawRectangleLines(static_cast<int>(box.x), static_cast<int>(box.y), static_cast<int>(box.width), static_cast<int>(box.height), DARKGRAY);
	}
	else {
		DrawRectangleLines(static_cast<int>(box.x), static_cast<int>(box.y), static_cast<int>(box.width), static_cast<int>(box.height), GRAY);
	}
	DrawText(buffer.c_str(), static_cast<int>(box.x) + fontSize / 4, static_cast<int>(box.y) + fontSize / 4, fontSize, BLACK);
	return retVal;
}

void ShowInfo(double zoom, dvec2* juliaC) {
	constexpr int fontSize = 20;
	constexpr int padding = 5;
	int currentHeight = 10;
	// show zoom level
	DrawText(TextFormat("Zoom level: %f", zoom), padding, currentHeight, fontSize, WHITE);
	currentHeight += fontSize + padding + padding;

	// show controls
	static std::vector<const char*> texts;
	static bool once = true;
	if (once) {
		once = false;
		texts.push_back("1 - Inc julia Re part");
		texts.push_back("2 - Dec julia Re part");
		texts.push_back("3 - Inc julia Im part");
		texts.push_back("4 - Dec julia Im part");
		texts.push_back("SPACE - Switch between modes");
		texts.push_back("G - Switch color modes");
		texts.push_back("WASD - Move");
		texts.push_back("SCROLL - Zoom");
		texts.push_back("+- - Inc/Dec zoom speed");
		texts.push_back("LSHIFT - Slow down zoom/inc/dec");
		texts.push_back("R - Reload shader");
		texts.push_back("F - Reset camera");
	}
	for (size_t i = 0; i < texts.size(); i++) {
		DrawText(texts[i], padding, padding + currentHeight, fontSize, WHITE);
		currentHeight += fontSize + padding;
	}

	// julia inputs
	static std::string buff1;
	InputBoxFloat(buff1, {static_cast<float>(padding), static_cast<float>(currentHeight)},
		{fontSize * 10, fontSize + padding}, fontSize, &(juliaC->x));
	currentHeight += fontSize + padding;
	static std::string buff2;
	InputBoxFloat(buff2, {static_cast<float>(padding), static_cast<float>(currentHeight)},
		{fontSize * 10, fontSize + padding}, fontSize, &(juliaC->y));
}

int main() {
	constexpr int windowWidth = 1000, windowHeight = 800;
	SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
	InitWindow(windowWidth, windowHeight, "Fractals");
	SetTargetFPS(120);

	Shader shader;
	int shaderScreenSize;
	int shaderCenterOffset;
	int shaderMoveOffset;
	int shaderZoom;
	int shaderJuliaC;
	int shaderMode;
	int shaderColorMode;

	auto screenResized = [&]() {
		const dvec2 screenSize = {
			static_cast<double>(GetScreenWidth()),
			static_cast<double>(GetScreenHeight())
		};
		SetShaderValue(shader, shaderScreenSize, &screenSize, SHADER_UNIFORM_DVEC2);

		const double ndcX = screenSize.x / std::min(screenSize.x, screenSize.y) * 2.0 - 1.0;
		const double ndcY = screenSize.y / std::min(screenSize.x, screenSize.y) * 2.0 - 1.0;
		const dvec2 centerOffset = {
			1.0 - ndcX,
			1.0 - ndcY,
		};
		SetShaderValue(shader, shaderCenterOffset, &centerOffset, SHADER_UNIFORM_DVEC2);
	};

	auto initShader = [&]() {
		shader = LoadShader(nullptr, "../shaders/fractals.frag");
		shaderScreenSize = GetShaderLocation(shader, "screenSize");
		shaderCenterOffset = GetShaderLocation(shader, "centerOffset");
		shaderMoveOffset = GetShaderLocation(shader, "moveOffset");
		shaderZoom = GetShaderLocation(shader, "zoom");
		shaderJuliaC = GetShaderLocation(shader, "juliaC");
		shaderMode = GetShaderLocation(shader, "mode");
		shaderColorMode = GetShaderLocation(shader, "colorMode");
		screenResized();
	};
	initShader();

	RenderTexture2D texture;
	InitTexture(&texture);

	dvec2 moveOffset{0, 0};
	double zoom{1};
	double zoomSpeed = 0.5;
	dvec2 juliaC{0, 0};
	int mode = 1;
	int colorMode = 0;
	while (!WindowShouldClose()) {
		if (IsWindowResized()) {
			InitTexture(&texture);
			screenResized();
		}

		// input
		static bool showInfo = false;
		if (IsKeyPressed(KEY_Q)) showInfo = !showInfo;
		if (IsKeyPressed(KEY_R)) {
			UnloadShader(shader);
			initShader();
		}
		if (IsKeyPressed(KEY_F)) {
			zoom = 0.5;
			moveOffset = {0, 0};
		}
		const double speed = 1.0 / zoom * static_cast<double>(GetFrameTime());
		double juliaSpeed = static_cast<double>(GetFrameTime());
		if (IsKeyDown(KEY_LEFT_SHIFT)) juliaSpeed *= 0.25;
		if (IsKeyPressed(KEY_SPACE)) {
			mode = mode == 1 ? 0 : 1;
		}
		if (IsKeyPressed(KEY_G)) {
			colorMode = colorMode == 1 ? 0 : 1;
		}
		if (IsKeyDown(KEY_ONE)) juliaC.x += juliaSpeed;
		if (IsKeyDown(KEY_TWO)) juliaC.x -= juliaSpeed;
		if (IsKeyDown(KEY_THREE)) juliaC.y += juliaSpeed;
		if (IsKeyDown(KEY_FOUR)) juliaC.y -= juliaSpeed;

		if (IsKeyDown(KEY_A)) moveOffset.x -= speed;
		if (IsKeyDown(KEY_D)) moveOffset.x += speed;
		if (IsKeyDown(KEY_W)) moveOffset.y += speed;
		if (IsKeyDown(KEY_S)) moveOffset.y -= speed;
		double scrollDiff = GetMouseWheelMove();
		if (IsKeyDown(KEY_LEFT_SHIFT)) scrollDiff *= 0.25;
		if (IsKeyDown(KEY_KP_ADD)) zoomSpeed += 0.01;
		if (IsKeyDown(KEY_KP_SUBTRACT)) zoomSpeed -= 0.01;
		if (scrollDiff != 0.f) {
			double oldMouseWorldX = static_cast<double>(GetMouseX());
			double oldMouseWorldY = static_cast<double>(GetMouseY());
			ScreenToWorld(&oldMouseWorldX, &oldMouseWorldY, zoom, moveOffset);

			zoom += zoomSpeed * scrollDiff * (zoom / 10.0);
			zoom = zoom < 0.1 ? 0.1 : zoom;

			double newMouseWorldX = static_cast<double>(GetMouseX());
			double newMouseWorldY = static_cast<double>(GetMouseY());
			ScreenToWorld(&newMouseWorldX, &newMouseWorldY, zoom, moveOffset);
			moveOffset.x -= newMouseWorldX - oldMouseWorldX;
			moveOffset.y += newMouseWorldY - oldMouseWorldY;
		}

		SetShaderValue(shader, shaderMoveOffset, &moveOffset, SHADER_UNIFORM_DVEC2);
		SetShaderValue(shader, shaderZoom, &zoom, SHADER_UNIFORM_DOUBLE);
		SetShaderValue(shader, shaderJuliaC, &juliaC, SHADER_UNIFORM_DVEC2);
		SetShaderValue(shader, shaderMode, &mode, SHADER_UNIFORM_INT);
		SetShaderValue(shader, shaderColorMode, &colorMode, SHADER_UNIFORM_INT);

		BeginTextureMode(texture);
		ClearBackground(BLACK);
		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
		EndTextureMode();

		BeginDrawing();
		ClearBackground(BLACK);
		BeginShaderMode(shader);
		DrawTextureEx(texture.texture, {0.0f, 0.0f}, 0.0f, 1.0f, WHITE);
		EndShaderMode();
		if (showInfo) ShowInfo(zoom, &juliaC);
		EndDrawing();
		// toggle fullscreen
		static int windowWidthOld = GetScreenWidth();
		static int windowHeightOld = GetScreenHeight();
		if (IsKeyReleased(KEY_F11)) {
			if (IsWindowFullscreen()) {
				ToggleFullscreen();
				SetWindowSize(windowWidthOld, windowHeightOld);
			}
			else {
				SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
				ToggleFullscreen();
			}
		}
	}

	UnloadShader(shader);
	CloseWindow();

	return 0;
}
