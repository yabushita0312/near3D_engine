﻿#include "sub.hpp"

bool MainClass::write_setting(void) {
	bool out = false;
	int x = 0;//1920 - 700;
	int y = 1080 - 580;

	int selpos[6] = { 0 }, selaim_o[6] = { 0 }, selaim[6] = { 0 }, selpadd[6] = {0};
	const auto font72 = FontHandle::Create(x_r(72), y_r(72 / 3), DX_FONTTYPE_NORMAL);

	for (int i = 0; i < 6; i++) {
		selaim_o[i] = selaim[i];
	}
	selaim[0] = 600 * USE_YSync;
	selaim[1] = 600 * int(frate) / 120;
	selaim[2] = 600 * USE_windowmode;
	selaim[3] = 600 * int(se_vol*100.f) / 100;
	selaim[4] = 600 * int(bgm_vol*100.f) / 100;
	selaim[5] = 600 * use_pad;
	for (int i = 0; i < 6; i++) {
		selpadd[i] = int(sqrt(2 * abs(selaim[i] - selaim_o[i]))) * ((selaim[i] - selaim_o[i] >= 0) ? 1 : -1);
	}

	int p = 0;
	while (ProcessMessage() == 0) {
		const auto waits = GetNowHiPerformanceCount();

		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
			out = true;
			break;
		}
		if (CheckHitKey(KEY_INPUT_SPACE) != 0)
			break;

		if (CheckHitKey(KEY_INPUT_P) != 0) {
			if (p == 0) {
				USE_YSync = false;
				frate = 30.f;
				USE_windowmode = true;
				se_vol = 0.0;
				bgm_vol = 0.0;
				use_pad = false;
				for (int i = 0; i < 6; i++) {
					selaim_o[i] = selaim[i];
				}
				selaim[0] = 600 * USE_YSync;
				selaim[1] = 600 * int(frate) / 120;
				selaim[2] = 600 * USE_windowmode;
				selaim[3] = 600 * int(se_vol*100.f) / 100;
				selaim[4] = 600 * int(bgm_vol*100.f) / 100;
				selaim[5] = 600 * use_pad;
				for (int i = 0; i < 6; i++) {
					selpadd[i] = int(sqrt(2 * abs(selaim[i] - selaim_o[i]))) * ((selaim[i] - selaim_o[i] >= 0) ? 1 : -1);
				}
				p = 1;
			}
		}
		else {
			p = 0;
		}

		
		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();

		font72.DrawStringFormat(0, y_r(72 * 0), GetColor(255, 255, 255), "press space", 0);

		DrawLine(x_r(x), y_r(y + 72 * 1 + 71), x_r(x + 600), y_r(y + 72 * 1 + 71), GetColor(128, 128, 128));
		DrawLine(x_r(x), y_r(y + 72 * 1 + 71), x_r(x + selpos[0]), y_r(y + 72 * 1 + 71), GetColor(0, 255, 0));//600 * USE_YSync
		font72.DrawStringFormat(x_r(x), y_r(y + 72 * 1), GetColor(255, 255, 255), "Sync    : %s", USE_YSync ? "true" : "false");
		DrawLine(x_r(x), y_r(y + 72 * 2 + 71), x_r(x + 600), y_r(y + 72 * 2 + 71), GetColor(128, 128, 128));
		DrawLine(x_r(x), y_r(y + 72 * 2 + 71), x_r(x + selpos[1]), y_r(y + 72 * 2 + 71), GetColor(0, 255, 0));//600 * int(frate) / 120
		font72.DrawStringFormat(x_r(x), y_r(y + 72 * 2), GetColor(255, 255, 255), "Fps     : %3.0f", frate);
		DrawLine(x_r(x), y_r(y + 72 * 3 + 71), x_r(x + 600), y_r(y + 72 * 3 + 71), GetColor(128, 128, 128));
		DrawLine(x_r(x), y_r(y + 72 * 3 + 71), x_r(x + selpos[2]), y_r(y + 72 * 3 + 71), GetColor(0, 255, 0));//600 * USE_windowmode
		font72.DrawStringFormat(x_r(x), y_r(y + 72 * 3), GetColor(255, 255, 255), "Window  : %s", USE_windowmode ? "true" : "false");
		DrawLine(x_r(x), y_r(y + 72 * 4 + 71), x_r(x + 600), y_r(y + 72 * 4 + 71), GetColor(128, 128, 128));
		DrawLine(x_r(x), y_r(y + 72 * 4 + 71), x_r(x + selpos[3]), y_r(y + 72 * 4 + 71), GetColor(0, 255, 0));//600 * int(se_vol*100.f) / 100
		font72.DrawStringFormat(x_r(x), y_r(y + 72 * 4), GetColor(255, 255, 255), "vol se  : %3.0f", se_vol*100.f);
		DrawLine(x_r(x), y_r(y + 72 * 5 + 71), x_r(x + 600), y_r(y + 72 * 5 + 71), GetColor(128, 128, 128));
		DrawLine(x_r(x), y_r(y + 72 * 5 + 71), x_r(x + selpos[4]), y_r(y + 72 * 5 + 71), GetColor(0, 255, 0));//600 * int(bgm_vol*100.f) / 100
		font72.DrawStringFormat(x_r(x), y_r(y + 72 * 5), GetColor(255, 255, 255), "vol bgm : %3.0f", bgm_vol*100.f);
		DrawLine(x_r(x), y_r(y + 72 * 6 + 71), x_r(x + 600), y_r(y + 72 * 6 + 71), GetColor(128, 128, 128));
		DrawLine(x_r(x), y_r(y + 72 * 6 + 71), x_r(x + selpos[5]), y_r(y + 72 * 6 + 71), GetColor(0, 255, 0));//600 * use_pad
		font72.DrawStringFormat(x_r(x), y_r(y + 72 * 6), GetColor(255, 255, 255), "GamePad : %s", use_pad ? "true" : "false");

		for (int i = 0; i < 6; i++) {
			if (selpadd[i] > 0) {
				selpos[i] += selpadd[i];
				selpadd[i]--;
			}
			else if (selpadd[i] < 0) {
				selpos[i] += selpadd[i];
				selpadd[i]++;
			}
			else {
				selpos[i] = selaim[i];
			}
		}
		Screen_Flip(waits);
	}
	if (!out) {
		std::ofstream outputfile("data/setting.txt");
		outputfile << "YSync(1or0)=" + std::to_string(USE_YSync) + "\n";
		outputfile << "fps(30or60or120)=" + std::to_string(frate) + "\n";
		outputfile << "windowmode(1or0)=" + std::to_string(USE_windowmode) + "\n";
		outputfile << "se_vol(100~0)=" + std::to_string(se_vol * 100.f) + "\n"; //
		outputfile << "bgm_vol(100~0)=" + std::to_string(bgm_vol * 100.f) + "\n"; //
		outputfile << "use_gamepad(1or0)=" + std::to_string(use_pad) + "\n"; //
		outputfile.close();
	}
	return out;
}
MainClass::MainClass(void) {
	using namespace std::literals;
	SetOutApplicationLogValidFlag(FALSE); /*log*/
	const auto mdata = FileRead_open("data/setting.txt", FALSE);
	{
		USE_YSync = bool(getparam_i(mdata));
		if (USE_YSync) {
			frate = 60.f;
			getparam_f(mdata);
		}
		else {
			frate = getparam_f(mdata);
		}
		USE_windowmode = bool(getparam_i(mdata));
		se_vol = getparam_f(mdata) / 100.f;
		bgm_vol = getparam_f(mdata) / 100.f;
		use_pad = bool(getparam_i(mdata));
	}
	FileRead_close(mdata);
	
	//SetFullScreenResolutionMode(DX_FSRESOLUTIONMODE_NATIVE);

	//SetWindowStyleMode(4);			    /**/
	//SetWindowUserCloseEnableFlag(FALSE);		    /*alt+F4対処*/
	SetMainWindowText("near3D");			    /*name*/
	SetAeroDisableFlag(TRUE);			    /**/
	SetUseDirect3DVersion(DX_DIRECT3D_9);		    /*directX ver*/
	SetEnableXAudioFlag(FALSE);			    /**/
	Set3DSoundOneMetre(1.0f);			    /*3Dsound*/
	SetGraphMode(dispx, dispy, 32);			    /*解像度*/
	SetWaitVSyncFlag(USE_YSync);			    /*垂直同期*/
	ChangeWindowMode(USE_windowmode);		    /*窓表示*/
	SetFullScreenScalingMode(DX_FSSCALINGMODE_NEAREST);/**/
	DxLib_Init();					    /*init*/
	SetAlwaysRunFlag(TRUE);				    /*background*/
	//SetSysCommandOffFlag(TRUE)				//強制ポーズ対策()
}
MainClass::~MainClass(void) {
	DxLib_End();
}
void MainClass::Screen_Flip(void) {
	ScreenFlip();
}
void MainClass::Screen_Flip(LONGLONG waits) {
	ScreenFlip();
	if (!USE_YSync)
		while (GetNowHiPerformanceCount() - waits < 1000000.0f / frate) {}
}

Draw_lookdown::Draw_lookdown(){
	zcon.resize(siz);
	for (auto& z : zcon)
		z.resize(siz);
}
Draw_lookdown::~Draw_lookdown(){
	for (auto& z : zcon)
		z.clear();
	zcon.clear();
}
void Draw_lookdown::draw_wall(int UorL, con cont){
	if (cont.hight == 0)
		UorL = -1;

	const auto oo_1 = getpos(cont.sx + cont.size * cont.px, cont.sy + cont.size * cont.py, cont.hight, camhigh);
	const auto lo_1 = getpos(cont.sx + cont.size * cont.px + cont.size, cont.sy + cont.size * cont.py, cont.hight, camhigh);
	const auto ol_1 = getpos(cont.sx + cont.size * cont.px, cont.sy + cont.size * cont.py + cont.size, cont.hight, camhigh);
	const auto ll_1 = getpos(cont.sx + cont.size * cont.px + cont.size, cont.sy + cont.size * cont.py + cont.size, cont.hight, camhigh);

	const auto oo_0 = getpos(cont.sx + cont.size * cont.px, cont.sy + cont.size * cont.py, 0, camhigh);
	const auto lo_0 = getpos(cont.sx + cont.size * cont.px + cont.size, cont.sy + cont.size * cont.py, 0, camhigh);
	const auto ol_0 = getpos(cont.sx + cont.size * cont.px, cont.sy + cont.size * cont.py + cont.size, 0, camhigh);
	const auto ll_0 = getpos(cont.sx + cont.size * cont.px + cont.size, cont.sy + cont.size * cont.py + cont.size, 0, camhigh);

	switch (UorL) {
	case 0:
		if (oo_0.y < oo_1.y) {
			DrawModiGraph(
				oo_1.x, oo_1.y,
				lo_1.x, lo_1.y,
				lo_0.x, lo_0.y,//基準
				oo_0.x, oo_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 1:
		if (ol_1.x >= ol_0.x) {
			DrawModiGraph(
				ol_1.x, ol_1.y,
				oo_1.x, oo_1.y,
				oo_0.x, oo_0.y,//基準
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 2:
		if (ol_0.y > ol_1.y) {
			DrawModiGraph(
				ol_1.x, ol_1.y,
				ll_1.x, ll_1.y,
				ll_0.x, ll_0.y,//基準
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 3:
		if (ll_0.x >= ll_1.x) {
			DrawModiGraph(
				ll_1.x, ll_1.y,
				lo_1.x, lo_1.y,
				lo_0.x, lo_0.y,//基準
				ll_0.x, ll_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 4:
		DrawTriangle(
			oo_1.x, oo_1.y,
			lo_1.x, lo_1.y,
			ll_1.x, ll_1.y,
			GetColor(128 - int(128.f*cont.hight / camhigh), 128 - int(128.f*cont.hight / camhigh), 128 - int(128.f*cont.hight / camhigh))
			//GetColor(255,255,255)
			, TRUE
		);
		DrawTriangle(
			oo_1.x, oo_1.y,
			ol_1.x, ol_1.y,
			ll_1.x, ll_1.y,
			GetColor(128 - int(128.f*cont.hight / camhigh), 128 - int(128.f*cont.hight / camhigh), 128 - int(128.f*cont.hight / camhigh))
			//GetColor(255,255,255)
			, TRUE
		);
		break;
	case 5://上◢
		if (lo_0.y < lo_1.y) {
			DrawModiGraph(
				lo_1.x, lo_1.y,
				lo_1.x, lo_1.y,
				lo_0.x, lo_0.y,
				oo_0.x, oo_0.y,
				cont.graphhandle, TRUE);
		}
		break;
	case 6://下◢
		if (ll_0.y > ll_1.y) {
			DrawModiGraph(
				ll_1.x, ll_1.y,
				ll_1.x, ll_1.y,
				ll_0.x, ll_0.y,//基準
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 7://上◣
		if (oo_0.y < oo_1.y) {
			DrawModiGraph(
				oo_1.x, oo_1.y,
				oo_1.x, oo_1.y,
				lo_0.x, lo_0.y,
				oo_0.x, oo_0.y,
				cont.graphhandle, TRUE);
		}
		break;
	case 8://下◣
		if (ol_0.y > ol_1.y) {
			DrawModiGraph(
				ol_1.x, ol_1.y,
				ol_1.x, ol_1.y,
				ll_0.x, ll_0.y,//基準
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 9://左◥
		if (ol_1.x >= oo_1.x) {
			DrawModiGraph(
				oo_1.x, oo_1.y,
				oo_1.x, oo_1.y,
				oo_0.x, oo_0.y,
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 10://右◥
		if (lo_1.x >= ll_1.x) {
			DrawModiGraph(
				lo_1.x, lo_1.y,
				lo_1.x, lo_1.y,
				lo_0.x, lo_0.y,
				ll_0.x, ll_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 11://左◢
		if (ol_1.x >= oo_1.x) {
			DrawModiGraph(
				ol_1.x, ol_1.y,
				ol_1.x, ol_1.y,
				oo_0.x, oo_0.y,
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 12://右◢
		if (lo_1.x >= ll_1.x) {
			DrawModiGraph(
				ll_1.x, ll_1.y,
				ll_1.x, ll_1.y,
				lo_0.x, lo_0.y,
				ll_0.x, ll_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 13:
		if (oo_0.y < ll_1.y) {
			DrawModiGraph(
				ol_1.x, ol_1.y,
				ll_1.x, ll_1.y,
				lo_0.x, lo_0.y,
				oo_0.x, oo_0.y,
				cont.graphhandle, TRUE);
		}
		break;
	case 14:
		if (oo_0.x < ll_1.x) {
			DrawModiGraph(
				ll_1.x, ll_1.y,
				lo_1.x, lo_1.y,
				oo_0.x, oo_0.y,
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 15:
		if (ol_0.y > lo_1.y) {
			DrawModiGraph(
				oo_1.x, oo_1.y,
				lo_1.x, lo_1.y,
				ll_0.x, ll_0.y,//基準
				ol_0.x, ol_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	case 16:
		if (lo_0.x > ol_1.x) {
			DrawModiGraph(
				ol_1.x, ol_1.y,
				oo_1.x, oo_1.y,
				lo_0.x, lo_0.y,
				ll_0.x, ll_0.y,//基準
				cont.graphhandle, TRUE);
		}
		break;
	default://床
		DrawTriangle(
			oo_0.x, oo_0.y,
			lo_0.x, lo_0.y,
			ll_0.x, ll_0.y,
			GetColor(0, 255, 0)
			, TRUE
		);
		DrawTriangle(
			oo_0.x, oo_0.y,
			ol_0.x, ol_0.y,
			ll_0.x, ll_0.y,
			GetColor(0, 255, 0)
			, TRUE
		);

		break;
	}
}
void Draw_lookdown::drw_rect(con cont){
	draw_wall(1, cont);	//横(左)
	draw_wall(3, cont);	//横(右)
	draw_wall(0, cont);	//縦(上)
	draw_wall(2, cont);	//縦(下)
	draw_wall(4, cont);		//天井
}
void Draw_lookdown::drw_prism(con cont){
	switch (cont.use) {
	case 0://上
		draw_wall(13, cont);	//縦(上)
		draw_wall(11, cont);	//横(左)
		draw_wall(2, cont);	//縦(下)
		draw_wall(12, cont);	//横(右)
		break;
	case 1://左
		draw_wall(5, cont);	//縦(上)
		draw_wall(14, cont);	//横(左)
		draw_wall(6, cont);	//縦(下)
		draw_wall(3, cont);	//横(右)
		break;
	case 2://下
		draw_wall(0, cont);	//縦(上)
		draw_wall(9, cont);	//横(左)
		draw_wall(15, cont);	//縦(下)
		draw_wall(10, cont);	//横(右)
		break;
	case 3://右
		draw_wall(7, cont);	//縦(上)
		draw_wall(1, cont);	//横(左)
		draw_wall(8, cont);	//縦(下)
		draw_wall(16, cont);	//横(右)
		break;
	}

}
void Draw_lookdown::put_drw(void){
	const auto cam = getpos(dispx / 2, dispy/2 , 0, camhigh);
	//DRAW
	for (int y = 1; y < siz; ++y) {
		for (int x = siz - 1; x >= 0; --x) {
			auto& z = zcon[x][y];
			if (z.dist2.x >= cam.x && z.dist1.y <= cam.y) {
				if (z.use == -1)
					drw_rect(z);
				else
					drw_prism(z);
			}
		}
		for (int x = 0; x < siz; ++x) {
			auto& z = zcon[x][y];
			if (z.dist1.x <= cam.x && z.dist1.y <= cam.y) {
				if (z.use == -1)
					drw_rect(z);
				else
					drw_prism(z);
			}
		}
	}
	for (int y = siz - 1; y >= 0; --y) {
		for (int x = siz - 1; x >= 0; --x) {
			auto& z = zcon[x][y];
			if (z.dist2.x >= cam.x && z.dist2.y >= cam.y) {
				if (z.use == -1)
					drw_rect(z);
				else
					drw_prism(z);
			}
		}
		for (int x = 0; x < siz; ++x) {
			auto& z = zcon[x][y];
			if (z.dist1.x <= cam.x && z.dist2.y >= cam.y) {
				if (z.use == -1)
					drw_rect(z);
				else
					drw_prism(z);
			}
		}
	}
}
void Draw_lookdown::set_drw_rect(int sx, int sy, int px, int py, int size, int hight, int graphhandle){
	auto& z = zcon[px][py];
	z.dist1 = getpos(sx + size * px, sy + size * py, hight, camhigh);
	z.dist2 = getpos(sx + size * px + size, sy + size * py + size, hight, camhigh);
	z.use = -1;
	z.sx = sx;
	z.sy = sy;
	z.px = px;//box
	z.py = py;//box
	z.size = size;
	z.hight = hight;
	z.graphhandle = graphhandle;
}
void Draw_lookdown::set_drw_prism(int UorL, int sx, int sy, int px, int py, int size, int hight, int graphhandle){
	auto& z = zcon[px][py];
	z.dist1 = getpos(sx + size * px, sy + size * py, hight, camhigh);
	z.dist2 = getpos(sx + size * px + size, sy + size * py + size, hight, camhigh);
	z.use = std::clamp(UorL, 0, 3);
	z.sx = sx;
	z.sy = sy;
	z.px = px;//box
	z.py = py;//box
	z.size = size;
	z.hight = hight;
	z.graphhandle = graphhandle;
}

void UIS::put_way(void) {
	waypoint = GetNowHiPerformanceCount();
	seldeb = 0;
}
void UIS::end_way(void) {
	if (seldeb < 6)
		waydeb[seldeb++] = (float)(GetNowHiPerformanceCount() - waypoint) / 1000.0f;
}
void UIS::debug(float fps, float time) {
	deb[0][0] = time;
	for (size_t j = std::size(deb) - 1; j >= 1; --j) {
		deb[j][0] = deb[j - 1][0];
	}
	for (size_t i = 0; i < std::size(waydeb); ++i) {
		if (seldeb - 1 <= i) {
			waydeb[i] = waydeb[seldeb - 1];
		}
		deb[0][i + 1] = waydeb[i];
		for (size_t j = std::size(deb) - 1; j >= 1; --j) {
			deb[j][i + 1] = deb[j - 1][i + 1];
		}
	}
	const auto c_ffff00 = GetColor(255, 255, 0);
	DrawBox(100, 100 + 0, 100 + 60 * 5, 100 + 200, GetColor(255, 0, 0), FALSE);
	for (int j = 0; j < 60 - 1; ++j) {
		for (int i = 0; i < 6; ++i) {
			DrawLine(100 + j * 5, 100 + (int)(200.f - deb[j][i + 1] * 10.f), 100 + (j + 1) * 5, 100 + (int)(200.f - deb[j + 1][i + 1] * 10.f), GetColor(50, 128 + 127 * i / 6, 50));
		}
		DrawLine(100 + j * 5, 100 + (int)(200.f - deb[j][0] * 10.f), 100 + (j + 1) * 5, 100 + (int)(200.f - deb[j + 1][0] * 10.f), c_ffff00);
	}
	const auto c_ffffff = GetColor(255, 255, 255);
	DrawLine(100, 100 + 200 - 166, 100 + 60 * 5, 100 + 200 - 166, GetColor(0, 255, 0));
	DrawFormatString(100, 100 + 0, c_ffffff, "%05.2ffps ( %.2fms)(total %.2fms)", fps, time, 1000.0f / fps);

	DrawFormatString(100, 100 + 18, c_ffffff, "%d(%.2fms)", 0, waydeb[0]);
	for (size_t j = 1; j < std::size(waydeb); ++j) {
		DrawFormatString(100, int(100 + 18 + j * 18), c_ffffff, "%d(%.2fms)", j, waydeb[j] - waydeb[j - 1u]);
	}
}

Draw_fps::Draw_fps() {
	campos = { 0, 0, 0 };
	camvec = { 0, 0, 100 };
	fov = 90;
}

Draw_fps::~Draw_fps() {
	wcon.clear();
	lcon.clear();
}

void Draw_fps::set_cam(pos3D cams, int xrad, int yrad, int zrad,int fovs){
	campos = cams;
	camvec.x = campos.x - sin_i(yrad, cos_i(xrad, 100));
	camvec.y = campos.y - sin_i(xrad, 100);
	camvec.z = campos.z - cos_i(yrad, cos_i(xrad, 100));
	fov = fovs;
}

void Draw_fps::draw_dot(int sx, int sy, int sz, bool hide){
	const auto d = getpos(sx, sy, sz);
	if(d.z>0)
		DrawPixel(d.x, d.y, GetColor(255, 255, 255));
}

void Draw_fps::draw_line(pos3D s, pos3D e, short dist, int chose){
	pos3D t = s, ot = t;
	bool on = false;
	bool off[8];
	int gg = -1;
	const int div1 = (getdist(s,e)/400);
	for (int i = 1; i <= div1; ++i) {
		auto d1 = getpos(t);
		int col = std::clamp(255 - 255 * getdist(t, campos) / dist, 0, 255);
		int distd = std::max(getdist(s, e), 1);

		ot = t;
		t = { s.x + (e.x - s.x)*i / div1,s.y + (e.y - s.y)*i / div1,s.z + (e.z - s.z)*i / div1 };
		auto d2 = getpos(t);

		if (col > 0) {
			for (int p = 0; p < 8; p++)
				off[p] = false;
			//*
			for (size_t w = 0; w < wsize && w < chose; w++) {

				if (getdist(wcon[w].mpos, campos) > 10000)
					continue;

				gethit_rect(wcon[w], d1, d2);
				for (int p = 0; p < 8; p++)
					off[p] = ((wcon[w].res & (1 << p)) != 0) ? true : off[p];//左
			}
			//*/
			if (!off[3]) {
				for (size_t w = 0; w < wsize && w < chose; w++) {
					if (((wcon[w].res & 2) != 0)) {
						if (d2.z >= 0) {
							d2.x = wcon[w].ep.x;
							d2.y = wcon[w].ep.y;
						}
					}
					if (((wcon[w].res & 4) != 0)) {
						if (d2.z >= 0) {
							d1.x = wcon[w].sp.x;
							d1.y = wcon[w].sp.y;
						}
					}
				}
			}

			gg = -1;
			if (off[1] || off[2]) { gg = 0; }
			if (off[3] && (off[5] || off[6] || off[7])) { gg = 1; }
			//*
			if (gg == 0) {
				if (d2.z > 0) {
					int col = std::clamp(255 - 255 * getdist(t, campos) / dist, 0, 255);
					int distd = std::max(getdist(s, e), 1);
					DrawLine(d1.x, d1.y, d2.x, d2.y,
						GetColor(
							col*int(abs(s.x - e.x)) / distd,
							col*int(abs(s.y - e.y)) / distd,
							col*int(abs(s.z - e.z)) / distd));
				}
			}
			//*/
			if (gg >= 0)
				continue;

			if (d2.z < 0) {
				if (on) {
					pos3D pt = ot;
					for (int j = 1; j <= div2; ++j) {
						const auto d1 = getpos(pt);
						int col = std::clamp(255 - 255 * getdist(pt, campos) / dist, 0, 255);
						int distd = std::max(getdist(s, e), 1);
						pt = { ot.x + (t.x - ot.x)*j / div2,ot.y + (t.y - ot.y)*j / div2,ot.z + (t.z - ot.z)*j / div2 };
						const auto d2 = getpos(pt);
						DrawLine(d1.x, d1.y, d2.x, d2.y, GetColor(
							col*int(abs(s.x - e.x)) / distd,
							col*int(abs(s.y - e.y)) / distd,
							col*int(abs(s.z - e.z)) / distd));
						if (d2.z < 0)
							break;
					}
					on = false;
				}
				else {
					pos3D tt = t;
					for (; i <= div1; ++i) {
						tt = { s.x + (e.x - s.x)*i / div1,s.y + (e.y - s.y)*i / div1,s.z + (e.z - s.z)*i / div1 };
						const auto d2 = getpos(tt);
						if (d2.z < 0)
							continue;
						t = { s.x + (e.x - s.x)*i / div1,s.y + (e.y - s.y)*i / div1,s.z + (e.z - s.z)*i / div1 };
						break;
					}
					pos3D pt = t;
					pos3D st = { s.x + (e.x - s.x)*(i - 1) / div1,s.y + (e.y - s.y)*(i - 1) / div1,s.z + (e.z - s.z)*(i - 1) / div1 };
					for (int j = div2; j > 0; --j) {
						const auto d1 = getpos(pt);
						int col = std::clamp(255 - 255 * getdist(pt, campos) / dist, 0, 255);
						int distd = std::max(getdist(s, e), 1);
						pt = { st.x + (t.x - st.x)*j / div2,st.y + (t.y - st.y)*j / div2,st.z + (t.z - st.z)*j / div2 };
						const auto d2 = getpos(pt);
						DrawLine(d1.x, d1.y, d2.x, d2.y, GetColor(
							col*int(abs(s.x - e.x)) / distd,
							col*int(abs(s.y - e.y)) / distd,
							col*int(abs(s.z - e.z)) / distd));
						if (d2.z < 0) {
							break;
						}
					}
					on = true;
				}
				continue;
			}
			else {
				on = true;
			}
			if (on) {
				DrawLine(d1.x, d1.y, d2.x, d2.y, GetColor(
					col*int(abs(s.x - e.x)) / distd,
					col*int(abs(s.y - e.y)) / distd,
					col*int(abs(s.z - e.z)) / distd));
			}
		}
	}
}

void Draw_fps::draw_wall(int sx, int sy, int sz, int ex, int ey, int ez, int chose, short dist) {
	pos3D e = { ex, ey, ez };
	pos3D s = { sx, sy, sz };

	if ((ez - sz)*(campos.x - sx) - (ex - sx)*(campos.z - sz) >= 0) {
		pos3D a = { ex, sy, ez };
		pos3D b = { sx, ey, sz };
		draw_line(s, a, dist, chose);
		draw_line(a, e, dist, chose);
		draw_line(e, b, dist, chose);
		draw_line(b, s, dist, chose);
	}
}

void Draw_fps::drw_rect(pos3D s, pos3D e, int chose, short dist) {
	draw_wall(e.x, s.y, e.z, s.x, e.y, e.z, chose, dist);//左
	draw_wall(e.x, s.y, s.z, e.x, e.y, e.z, chose, dist);//右
	draw_wall(s.x, s.y, e.z, s.x, e.y, s.z, chose, dist);//後
	draw_wall(s.x, s.y, s.z, e.x, e.y, s.z, chose, dist);//前
}

void Draw_fps::set_drw_line(pos3D s, pos3D e){
	pos3D t = s;
	const int div1 = getdist(s, e) / 400;//
	for (int i = 1; i <= div1; ++i) {
		auto d1 = getpos(t);
		t = { s.x + (e.x - s.x)*i / div1,s.y + (e.y - s.y)*i / div1,s.z + (e.z - s.z)*i / div1 };
		auto d2 = getpos(t);

		if (d2.z >= 0) {
			if (lsize >= lcon.size()) { lcon.resize(lsize + 1); }
			lcon[lsize].pos[0] = s;
			lcon[lsize].pos[1] = e;
			lsize++;
			break;
		}
	}
}
//lconとwconに貯めた描画物を一気に描画する
void Draw_fps::set_drw_line(int sx, int sy, int sz, int ex, int ey, int ez){
	const pos3D s = { sx,sy,sz };
	const pos3D e = { ex,ey,ez };
	set_drw_line(s, e);
}
void Draw_fps::set_drw_rect(pos3D s, pos3D e){
	pos3D m = { (s.x + e.x) / 2, (s.y + e.y) / 2, (s.z + e.z) / 2 };
	if (getdot(getsub(m, campos), getsub(camvec, campos)) > 0) {
		wsize++;
		if (wsize - 1 >= wcon.size()) { wcon.resize(wsize); }
		size_t i = 0;
		if (wsize > 1) {
			const auto b = getdist(m, campos);
			for (i = 0; i < wsize - 1; i++) {
				if (b < getdist(wcon[i].mpos, campos)) {
					for (int j = int(wsize) - 1; j > i; j--) {
						wcon[j].pos[0] = wcon[j - 1].pos[0];
						wcon[j].pos[1] = wcon[j - 1].pos[1];
						wcon[j].mpos = wcon[j - 1].mpos;
					}
					break;
				}
			}
		}
		wcon[i].pos[0] = s;
		wcon[i].pos[1] = e;
		wcon[i].mpos = m;
	}
}
void Draw_fps::set_drw_rect(int sx, int sy, int sz, int ex, int ey, int ez) {
	pos3D m = { (sx + ex) / 2, (sy + ey) / 2, (sz + ez) / 2 };
	if (getdot(getsub(m, campos), getsub(camvec, campos)) > 0) {
		wsize++;
		if (wsize - 1 >= wcon.size()) { wcon.resize(wsize); }
		size_t i = 0;
		if (wsize > 1) {
			const auto b = getdist(m, campos);
			for (i = 0; i < wsize - 1; i++) {
				if (b < getdist(wcon[i].mpos, campos)) {
					for (int j = int(wsize) - 1; j > i; j--) {
						wcon[j].pos[0] = wcon[j - 1].pos[0];
						wcon[j].pos[1] = wcon[j - 1].pos[1];
						wcon[j].mpos = wcon[j - 1].mpos;
					}
					break;
				}
			}
		}
		wcon[i].pos[0] = { sx, sy, sz };
		wcon[i].pos[1] = { ex, ey, ez };
		wcon[i].mpos = m;
	}
}
void Draw_fps::put_drw(void){
	for (size_t i = 0; i < lsize; i++)
		draw_line(lcon[i].pos[0], lcon[i].pos[1],5000);
	for (int i = 0; i < wsize; i++)
		drw_rect(wcon[i].pos[0], wcon[i].pos[1], i,7500);
}

void Draw_fps::end_drw(void){
	lsize = 0;
	wsize = 0;
}

