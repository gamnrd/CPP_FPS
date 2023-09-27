#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
using namespace std;

#include <Windows.h>

//Screen
int nScreenWidth = 120;
int nScreenHeight = 40;

//Player
float fPlayerX = 2.0f;
float fPlayerY = 2.0f;
float fPlayerA = 0.0f;
float fFOV = 3.14 / 4.0;
float fPlayerRotSpeed = 0.8f;
float fPlayerMovSpeed = 5.0f;

//Map
wstring map;
int nMapHeight = 16;
int nMapWidth = 16;

float fDepth = 16.0f;





int main()
{
	//Create screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Create Map
	//# = wall
	//. = space

	map += L"################";
	map += L"#...#..........#";
	map += L"#...#..#####...#";	
	map += L"#...#......#...#";
	map += L"#...#..#####...#";
	map += L"#...#.....#....#";
	map += L"#...###...###..#";
	map += L"#...........#..#";
	map += L"#...........#..#";
	map += L"####..#######..#";
	map += L"#..........#...#";
	map += L"#..######..#..##";
	map += L"#..#....#..#...#";
	map += L"#..#..###..#...#";
	map += L"#.....#....#...#";
	map += L"################";

	/* 30x30
	map += L"##############################";
	map += L"##......##..................##";
	map += L"##......##..##############..##";	
	map += L"##......##..........##......##";
	map += L"######..##..######..##..######";
	map += L"##......##......##..##......##";
	map += L"##..##########..##..######..##";
	map += L"##..............##......##..##";
	map += L"######..##########..######..##";
	map += L"##......##..........##......##";
	map += L"##..######..######..##..######";
	map += L"##..##......##......##......##";
	map += L"##..##..######..######......##";
	map += L"##......##..........##......##";
	map += L"##############################";
	*/

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();


	
	//Game Loop
	while (true)
	{
		//Get fps to modify player speed
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();






		////////////////Controls
	    //Move forwards
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;
			}
		}

		//Move backwards
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * fPlayerMovSpeed * fElapsedTime;
			}
		}

		//Rotate Left
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= fPlayerRotSpeed * fElapsedTime;

		//Rotate Right
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += fPlayerRotSpeed * fElapsedTime;






		////////////////Create screen to render
		for (int x = 0; x < nScreenWidth; x++)
		{
			//Calculate projected ray into the map
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bHitBoundary = false;

			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//Check if raycast is out of map bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true; fDistanceToWall = fDepth;
				}
				//else check within bounds
				else
				{
					if (map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;

						vector<pair<float, float>> p; //distance dot

						for (int tx = 0; tx < 2; tx++)
						{
							for (int ty = 0; ty < 2; ty++)
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

							sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });

							float fBound = 0.005f;
							if (acos(p.at(0).second) < fBound) bHitBoundary = true;
							if (acos(p.at(1).second) < fBound) bHitBoundary = true;
						}
					}
				}
			}

			//Check wall height based on distance
			int nCeiling = (float)(nScreenHeight / 2.0f) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			//Figure out shading of wall based on distance
			short nWallShade = ' ';
			short nFloorShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f)		nWallShade = 0x2588;
			else if (fDistanceToWall <= fDepth / 3.0f)	nWallShade = 0x2593;
			else if (fDistanceToWall <= fDepth / 2.0f)	nWallShade = 0x2592;
			else if (fDistanceToWall <= fDepth)			nWallShade = 0x2591;
			else										nWallShade = ' ';

			if (bHitBoundary)							nWallShade = 0x2591;


			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y < nCeiling)
				{
					screen[y * nScreenWidth + x] = ' ';
				}
				else if (y > nCeiling && y <= nFloor)
				{
					screen[y * nScreenWidth + x] = nWallShade;
				}
				else
				{
					//Shade floor based on distance
					float f = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (f < 0.25f)		nFloorShade = '#';
					else if (f < 0.5f)	nFloorShade = 'x';
					else if (f < 0.75f) nFloorShade = '.';
					else if (f < 0.9f)	nFloorShade = '-';
					else				nFloorShade = ' ';
					screen[y * nScreenWidth + x] = nFloorShade;
				}
			}

		}

		//Display in game stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		//Display Map
		for (int nx = 0; nx < nMapWidth; nx++)
		{
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		}

		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

		////////////////Render to screen to console
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}

	return 0;
}