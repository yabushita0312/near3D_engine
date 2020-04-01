﻿#pragma once

#ifndef INCLUDED_define_h_
#define INCLUDED_define_h_

#define NOMINMAX
#include "DxLib.h"
#include "useful.hpp"
#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"
#include <windows.h>
#include <fstream>
#include <string_view>
#include <optional>
#include <array>
#include <vector>
#include <utility>
#include <string_view>
#include <string>
#include <iostream>
#include <fstream>
#include <thread> 

#include <chrono>

class MainClass {
private:
	/*setting*/
	int refrate;
	bool USE_YSync;	       /*垂直同期*/
	int frate;	       /*fps*/
	int se_vol;	       /**/
	int bgm_vol;	       /**/
	bool use_pad;		/**/
	std::optional<LONGLONG> time;
public:
	/*setting*/
	inline const auto get_se_vol(void) { return se_vol; }
	inline const auto get_bgm_vol(void) { return bgm_vol; }
	inline const auto get_use_pad(void) { return use_pad; }
	bool write_setting(void);
	MainClass(void);
	~MainClass(void);
	/*draw*/
	void Start_Screen(void);
	void Screen_Flip(void);
	void Screen_Flip(LONGLONG waits);
};

class Draw{
private:
	//2Dベクトル関連
	class pos2D {
	public:
		int x;
		int y;
		//加算した値を返す
		inline pos2D operator+(pos2D o)noexcept {
			pos2D temp;
			temp.x = this->x + o.x;
			temp.y = this->y + o.y;
			return temp;
		}
		//加算する
		inline pos2D& operator+=(pos2D o)noexcept {
			this->x += o.x;
			this->y += o.y;
			return *this;
		}
		//減算した値を変えす
		inline pos2D operator-(pos2D o)noexcept {
			pos2D temp;
			temp.x = this->x - o.x;
			temp.y = this->y - o.y;
			return temp;
		}
		//減算する
		inline pos2D& operator-=(pos2D o)noexcept {
			this->x -= o.x;
			this->y -= o.y;
			return *this;
		}
		//距離の2乗を取得する
		inline int hydist()noexcept {
			return this->x*this->x + this->y*this->y;
		}
		// 内積
		inline int dot(pos2D* v2)noexcept {
			return this->x * v2->x + this->y * v2->y;
		}
		// 外積
		inline int cross(pos2D* v2)noexcept {
			return this->x * v2->y - this->y * v2->x;
		}
	};
	//線分衝突
	inline bool ColSeg(pos2D &pos1, pos2D &vec1, pos2D &pos2, pos2D &vec2) {
		auto Crs_v1_v2 = vec1.cross(&vec2);
		if (Crs_v1_v2 == 0) {
			return false;// 平行
		}
		pos2D v = pos2 - pos1;
		auto Crs_v_v1 = v.cross(&vec1);
		auto Crs_v_v2 = v.cross(&vec2);
		if (Crs_v_v2 < 0 || Crs_v_v2 > Crs_v1_v2 || Crs_v_v1 < 0 || Crs_v_v1 > Crs_v1_v2) {
			return false;// 交差X
		}
		return true;
	}
	//座標変換
	inline pos2D getpos(int xp, int yp, int high) {
		pos2D p;
		p.x = dispx / 2 + (camhigh * xp) / std::max(camhigh - high, 1);
		p.y = dispy / 2 + (camhigh * yp) / std::max(camhigh - high, 1);
		return p;
	}
private:
	const int camhigh = 256;
	const int tilesize = 32;
	struct con {
		std::array<pos2D, 4> top;
		std::array<pos2D, 4> floor;
		uint8_t use;// rect = -1 else prism = 0~3,4~7
		pos2D cpos;
		int bottom;
		int hight;
		GraphHandle* wallhandle;
		GraphHandle* floorhandle;
	};
	struct Status {
		size_t xp, yp;
		int bottom, hight;
		int wall_id, floor_id;
		int dir;
	};
	struct maps {
		int plx, ply;
		char wall_name[MAX_PATH];
		char floor_name[MAX_PATH];
		float light_yrad;
	};
	struct enemies {
		int xp, yp;
	};
	std::vector<Status> ans;
	std::vector<std::vector<con>> zcon;
	int xpos = 0, ypos = 0;
	float light_yrad=0.0f;
	std::vector<GraphHandle> walls;
	std::vector<GraphHandle> floors;
	int shadow_graph;
	int screen;
	struct Bonesdata {
		int parent;
		int xp, yp, zp;
		float xr, yr, zr;
		float xrad,yrad,zrad;
		float xdist,ydist,zdist;
		bool edit;
	};
	struct Animesdata {
		int time=0;
		std::vector<Bonesdata> bone;
	};
	struct Humans {
		std::vector<GraphHandle> Graphs;
		std::vector<Bonesdata> bone;
		std::vector<pair> sort;
		std::vector<std::vector<Animesdata>> anime;
		pos2D vtmp = { 0,0 };
		pos2D vec = { 0,0 };
		float yrad = 0.f;
		pos2D pos = { 0,0 };
		int animeframe=0;
		int animetime=1;
		int animesel=0;
	};
	std::vector<Humans> human;
	size_t player_id=0;
public:
	Draw(void){
		shadow_graph = MakeScreen(dispx, dispy, TRUE);
		screen = MakeScreen(dispx, dispy,FALSE);
	}
	~Draw(void){
		exit_map();
	}
	//mapエディター
	inline bool write_map(std::string mapname) {
		using namespace std::literals;
		std::fstream file;
		const auto font40 = FontHandle::Create(x_r(40), y_r(40 / 3), DX_FONTTYPE_ANTIALIASING);
		const auto font30 = FontHandle::Create(x_r(30), y_r(30 / 3), DX_FONTTYPE_ANTIALIASING);
		bool wallorfloor = false;
		size_t wofcnt = 0, floortex = 0, walltex = 0;
		size_t mscnt = 0, cslcnt = 0;
		//ダイアログ用
		static TCHAR strFile[MAX_PATH], cdir[MAX_PATH], *ansFile;
		static OPENFILENAME ofn = { 0 };
		GetCurrentDirectory(MAX_PATH, cdir);
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetMainWindowHandle();
		ofn.lpstrFilter =
			TEXT("Picture files {*.bmp}\0*.bmp\0")
			TEXT("Picture files {*.png}\0*.png\0");
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter = NULL;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = strFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST| OFN_NOCHANGEDIR;
		ofn.lpstrInitialDir = cdir;
		ofn.lpstrTitle = "カレントディレクトリより下層のファイルを指定してください";
		//
		std::vector<Status> n;
		maps mapdata;
		std::vector<enemies> e;
		size_t x_size = 0, y_size = 0;

		{
			size_t okcnt = 0, ngcnt = 0;
			bool read = false;
			while (ProcessMessage() == 0) {
				SetDrawScreen(DX_SCREEN_BACK);
				ClearDrawScreen();
				int mousex, mousey;
				GetMousePoint(&mousex, &mousey);

				DrawBox(x_r(960 - 320), y_r(540 - 180), x_r(960 + 320), y_r(540 + 180), GetColor(128, 128, 128), TRUE);
				font40.DrawString(x_r(960 - 320 + 40), y_r(540 - 180 + 60), "プリセットを読み込みますか?", GetColor(255, 255, 0));

				//OK
				{
					int xs = x_r(960 + 320 - 340);
					int ys = y_r(540 + 180 - 140);
					int xe = xs + x_r(300);
					int ye = ys + y_r(40);
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth("OK")) / 2, ys, "OK", GetColor(255, 255, 0));

						if (okcnt == 1) {
							okcnt = 2;
							read = true;
							break;
						}
						okcnt = std::min<size_t>(okcnt + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth("OK")) / 2, ys, "OK", GetColor(255, 0, 0));
						okcnt = 2;
					}
				}
				//NG
				{
					int xs = x_r(960 + 320 - 340);
					int ys = y_r(540 + 180 - 80);
					int xe = xs + x_r(300);
					int ye = ys + y_r(40);
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth("NG")) / 2, ys, "NG", GetColor(255, 255, 0));

						if (ngcnt == 1) {
							ngcnt = 2;
							break;
						}
						ngcnt = std::min<size_t>(ngcnt + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth("NG")) / 2, ys, "NG", GetColor(255, 0, 0));
						ngcnt = 2;
					}
				}
				ScreenFlip();
			}
			if(!read){
				//mapデータ1読み込み(マップチップ)
				{
					file.open(("data/Map/" + mapname + "/1.dat").c_str(), std::ios::binary | std::ios::in);
					do {
						n.resize(n.size() + 1);
						file.read((char*)&n.back(), sizeof(n.back()));
						x_size = std::max<size_t>(n.back().xp, x_size);
						y_size = std::max<size_t>(n.back().yp, y_size);
					} while (!file.eof());
					x_size++;
					y_size++;
					file.close();
					n.pop_back();
				}
				//mapデータ2読み込み(プレイヤー初期位置、使用テクスチャ指定)
				{
					file.open(("data/Map/" + mapname + "/2.dat").c_str(), std::ios::binary | std::ios::in);
					file.read((char*)&mapdata, sizeof(mapdata));
					file.close();
				}
				//mapデータ3読み込み(敵情報)
				{
					file.open(("data/Map/" + mapname + "/3.dat").c_str(), std::ios::binary | std::ios::in);
					do {
						e.resize(e.size() + 1);
						file.read((char*)&e.back(), sizeof(e.back()));
					} while (!file.eof());
					file.close();
					e.pop_back();
				}
			}
			//mapデータプリセット
			else {
				{
					//mapデータ1書き込み(マップチップ)
					x_size = 40;
					y_size = 40;
					{
						size_t upx = 0, dnx = 0, upy = 0, dny = 0;
						while (ProcessMessage() == 0) {
							SetDrawScreen(DX_SCREEN_BACK);
							ClearDrawScreen();
							int mousex, mousey;
							GetMousePoint(&mousex, &mousey);

							DrawBox(x_r(960 - 320), y_r(540 - 180), x_r(960 + 320), y_r(540 + 180), GetColor(128, 128, 128), TRUE);
							font40.DrawString(x_r(960 - 320 + 40), y_r(540 - 180 + 60), "マップのサイズは?", GetColor(255, 255, 0));

							{
								int xs = x_r(960 - 320 + 40 + 200);
								int xe = xs + x_r(40);
								//X
								font40.DrawStringFormat(xs - x_r(200), y_r(540 - 180 + 60 + 40 + 15), GetColor(255, 255, 0), "X : %d", x_size);
								{
									int ys = y_r(540 - 180 + 60 + 40);
									int ye = ys + y_r(30);
									if (inm(xs, ys, xe, ye)) {
										DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
										if (upx == 1) {
											upx = 2;
											x_size++;
										}
										upx = std::min<size_t>(upx + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
									}
									else {
										DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
										upx = 2;
									}
								}
								{
									int ys = y_r(540 - 180 + 60 + 40 + 40);
									int ye = ys + y_r(30);
									if (inm(xs, ys, xe, ye)) {
										DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
										if (dnx == 1) {
											dnx = 2;
											if (x_size > 1) {
												x_size--;
											}
										}
										dnx = std::min<size_t>(dnx + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
									}
									else {
										DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
										dnx = 2;
									}
								}
								//Y
								font40.DrawStringFormat(xs - x_r(200), y_r(540 - 180 + 60 + 40 + 100 + 15), GetColor(255, 255, 0), "Y : %d", y_size);
								{
									int ys = y_r(540 - 180 + 60 + 40 + 100);
									int ye = ys + y_r(30);
									if (inm(xs, ys, xe, ye)) {
										DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
										if (upy == 1) {
											upy = 2;
											y_size++;
										}
										upy = std::min<size_t>(upy + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
									}
									else {
										DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
										upy = 2;
									}
								}
								{
									int ys = y_r(540 - 180 + 60 + 40 + 100 + 40);
									int ye = ys + y_r(30);
									if (inm(xs, ys, xe, ye)) {
										DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
										if (dny == 1) {
											dny = 2;
											if (y_size > 1) {
												y_size--;
											}
										}
										dny = std::min<size_t>(dny + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
									}
									else {
										DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
										dny = 2;
									}
								}
							}
							{
								int xsz = x_r(280);
								int ysz = y_r(120);
								int xm = x_r(1100);
								int ym = y_r(540);

								if (x_size*ysz / xsz >= y_size) {
									ysz = xsz * int(y_size) / int(x_size);
								}
								else {
									xsz = ysz * int(x_size) / int(y_size);
								}
								DrawBox(xm - xsz / 2, ym - ysz / 2, xm + xsz / 2, ym + ysz / 2, GetColor(255, 255, 0), FALSE);

							}
							//終了
							{
								int xs = x_r(960 + 320 - 340);
								int ys = y_r(540 + 180 - 80);
								int xe = xs + x_r(300);
								int ye = ys + y_r(40);
								if (inm(xs, ys, xe, ye)) {
									DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
									font40.DrawString(xs + (xe - xs - font40.GetDrawWidth("OK")) / 2, ys, "OK", GetColor(255, 255, 0));

									if (mscnt == 1) {
										mscnt = 2;
										break;
									}
									mscnt = std::min<size_t>(mscnt + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
								}
								else {
									DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
									font40.DrawString(xs + (xe - xs - font40.GetDrawWidth("OK")) / 2, ys, "OK", GetColor(255, 0, 0));
									mscnt = 2;
								}
							}
							ScreenFlip();
						}

					}
					for (size_t y = 0; y < y_size; y++) {
						for (size_t x = 0; x < x_size; x++) {
							const int btm = 0;
							const int mid = 0;
							const int hig = 64;
							n.resize(n.size() + 1);
							n.back() = { x, y, btm,mid, 2, 2, -1 };
							//n.back() = { x, y, btm,hig, 2, 2, -1 };
						}
					}

					//(マップチップ)
					for (size_t y = 0; y < y_size; y += 5) {
						for (size_t x = 0; x < x_size; x += 5) {
							const int btm = 0;
							const int mid = 0;
							const int hig = 64;
							n[(x + 2u) + (y + 1u) * x_size] = { x + 2u, y + 1u, btm, hig, 2, 2, -1 };
							n[(x + 1u) + (y + 2u) * x_size] = { x + 1u, y + 2u, btm, hig, 2, 2, -1 };
							n[(x + 2u) + (y + 2u) * x_size] = { x + 2u, y + 2u, btm, hig, 2, 2, -1 };
							n[(x + 3u) + (y + 2u) * x_size] = { x + 3u, y + 2u, btm, hig, 2, 2, -1 };
							n[(x + 2u) + (y + 3u) * x_size] = { x + 2u, y + 3u, btm, hig, 2, 2, -1 };
							/*
							n[(x + 2u) + (y + 1u) * x_size] = { x + 2u, y + 1u, btm, hig, 2, 2, 0 };
							n[(x + 1u) + (y + 2u) * x_size] = { x + 1u, y + 2u, btm, hig, 2, 2, 1 };
							n[(x + 2u) + (y + 2u) * x_size] = { x + 2u, y + 2u, btm, hig, 2, 1, -1 };
							n[(x + 3u) + (y + 2u) * x_size] = { x + 3u, y + 2u, btm, hig, 2, 2, 3 };
							n[(x + 2u) + (y + 3u) * x_size] = { x + 2u, y + 3u, btm, hig, 2, 2, 2 };
							*/
						}
					}
				}
				{
					//mapデータ2書き込み(プレイヤー初期位置、使用テクスチャ指定)
					mapdata.plx = 32;
					mapdata.ply = 32;
					mapdata.light_yrad = deg2rad(0);
					strcpy_s(mapdata.wall_name, "data/Chip/Wall/1.bmp");
					strcpy_s(mapdata.floor_name, "data/Chip/Floor/1.bmp");
				}
				{
					//mapデータ3書き込み(敵情報)
					for (int i = 1; i < 8; i++) {
						e.resize(e.size() + 1);
						e.back().xp = 32 * 5 * i + 16;
						e.back().yp = 32 * 5 * i + 16;
					}
				}
			}
		}
		//エディター
		int hight_s = 64,bottom_s = 0;
		size_t upx = 2, dnx = 2, upy = 2, dny = 2;
		size_t undo = 2, redo = 2;
		bool save = false;
		std::list<std::vector<Status>> n_list;

		n_list.push_back(n);
		auto itr = n_list.end();

		while (ProcessMessage() == 0) {
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();

			int mousex, mousey;
			GetMousePoint(&mousex, &mousey);
			//マップ描画
			{
				for (auto& m : n) {
					int xs = dispy / 40 + int(m.xp * dispy * 38 / 40 / std::max(x_size, y_size));
					int ys = dispy / 40 + int(m.yp * dispy * 38 / 40 / std::max(x_size, y_size));
					int xe = dispy / 40 + int((m.xp + 1)*dispy * 38 / 40 / std::max(x_size, y_size));
					int ye = dispy / 40 + int((m.yp + 1)*dispy * 38 / 40 / std::max(x_size, y_size));

					if (inm(xs, ys, xe, ye)) {
						if (m.bottom != m.hight) {
							DrawBox(xs, ys, xe, ye, GetColor(255 * (camhigh - m.hight) / camhigh, 128 * (camhigh - m.hight) / camhigh, 0), TRUE);
						}
						else {
							DrawBox(xs, ys, xe, ye, GetColor(255 * (camhigh - m.hight) / camhigh, 128 * (camhigh - m.hight) / camhigh, 128 * (camhigh - m.hight) / camhigh), TRUE);
						}
						font40.DrawStringFormat(int(x_size*dispy / std::max(x_size, y_size)), y_r(40), GetColor(255, 255, 255), "(%03d,%03d)", m.xp, m.yp);

						if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0 || (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
							if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) {
								if (wallorfloor) {
									m.hight = hight_s;
									//壁
								}
								else {
									m.bottom = bottom_s;
									m.hight = m.bottom;
									//床
								}
								save = true;
							}else if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
								if (!wallorfloor) {
									m.hight = hight_s;
									m.bottom = m.hight;
									//壁
								}
								else {
									m.bottom = bottom_s;
									m.hight = m.bottom;
									//床
								}
								save = true;
							}
						}
						else {
							if (save) {
								if (itr == n_list.end()--) {
									n_list.push_back(n);
									itr = n_list.end()--;
								}
								else {
									itr++;
									n_list.insert(itr,n);
									n_list.erase(itr++, n_list.end());
									itr = n_list.end();
								}
								itr--;
								//nを保存
							}
							save = false;
						}
					}
					else {
						if (m.bottom != m.hight) {
							DrawBox(xs, ys, xe, ye, GetColor(255 * (camhigh - m.hight) / camhigh, 255 * (camhigh - m.hight) / camhigh, 0), TRUE);
						}
						else {
							DrawBox(xs, ys, xe, ye, GetColor(255 * (camhigh - m.hight) / camhigh, 255 * (camhigh - m.hight) / camhigh, 255 * (camhigh - m.hight) / camhigh), TRUE);
						}
					}

					if (m.bottom != m.hight) {
						DrawBox(xs, ys, xe, ye, GetColor(0, 0, 0), FALSE);
					}
				}

				DrawCircle(
					dispy / 40 + mapdata.plx * int(dispy * 38 / 40 / std::max(x_size, y_size)) / tilesize,
					dispy / 40 + mapdata.ply * int(dispy * 38 / 40 / std::max(x_size, y_size)) / tilesize,
					y_r(dispy * 38 / 40 / std::max(x_size, y_size)),
					GetColor(0, 255, 0));
				for (auto& m : e) {
					DrawCircle(
						dispy / 40 + m.xp * int(dispy * 38 / 40 / std::max(x_size, y_size)) / tilesize,
						dispy / 40 + m.yp * int(dispy * 38 / 40 / std::max(x_size, y_size)) / tilesize,
						y_r(dispy / std::max(x_size, y_size)),
						GetColor(255, 0, 0));
				}
			}
			//壁か床か
			{
				int xs = int(x_size * dispy / std::max(x_size, y_size));
				int ys = y_r(80);
				int xe = xs + x_r(400);
				int ye = ys + y_r(40);
				char buf[] = "選択タイルを変更";

				if (inm(xs, ys, xe, ye)) {
					DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 255, 0));

					if (wofcnt == 1) {
						wallorfloor ^= 1;
					}
					wofcnt = std::min<size_t>(wofcnt + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
				}
				else {
					DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 0, 0));
					wofcnt = 2;
				}
				font40.DrawString(xs, ye, wallorfloor ? "壁を選択中" : "床を選択中", GetColor(255, 0, 0));
			}
			//床テクスチャ
			{
				int xs = int(x_size * dispy / std::max(x_size, y_size));
				int ys = y_r(180);
				int xe = xs + x_r(400);
				int ye = ys + y_r(40);
				char buf[] = "床テクスチャ選択";

				if (inm(xs, ys, xe, ye)) {
					DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 255, 0));

					if (floortex == 1) {
						if (GetOpenFileName(&ofn)) {
							std::string str = strFile;
							if (str.find(cdir) != std::string::npos) {
								ansFile = &strFile[strlen(cdir) + 1];
								strcpy_s(mapdata.floor_name, ansFile);
							}
							else {
								strcpy_s(mapdata.floor_name, strFile);//フルパス
							}
						}
					}
					floortex = std::min<size_t>(floortex + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
				}
				else {
					DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 0, 0));
					floortex = 2;
				}
				font30.DrawString(xs, ye, mapdata.floor_name, GetColor(255, 0, 0));
			}
			//壁テクスチャ
			{
				int xs = int(x_size * dispy / std::max(x_size, y_size));
				int ys = y_r(280);
				int xe = xs + x_r(400);
				int ye = ys + y_r(40);
				char buf[] = "壁テクスチャ選択";

				if (inm(xs, ys, xe, ye)) {
					DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 255, 0));

					if (walltex == 1) {
						if (GetOpenFileName(&ofn)) {
							std::string str = strFile;
							if (str.find(cdir) != std::string::npos) {
								ansFile = &strFile[strlen(cdir) + 1];
								strcpy_s(mapdata.wall_name, ansFile);
							}
							else {
								strcpy_s(mapdata.wall_name, strFile);//フルパス
							}
						}
					}
					walltex = std::min<size_t>(walltex + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
				}
				else {
					DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 0, 0));
					walltex = 2;
				}
				font30.DrawString(xs, ye, mapdata.wall_name, GetColor(255, 0, 0));
			}
			//設定する高さ
			{
				int xs = int(x_size * dispy / std::max(x_size, y_size))+x_r(400);
				int xe = xs + x_r(40);
				//高
				font40.DrawStringFormat(xs - x_r(400), y_r(380+15), GetColor(255, 255, 0), "設定する高さ : %d", hight_s);
				{
					int ys = y_r(380);
					int ye = ys + y_r(30);
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						if (upx == 1) {
							upx = 2;
							if (hight_s < camhigh) {
								hight_s+=8;
							}
							else {
								hight_s = camhigh;
							}
						}
						upx = std::min<size_t>(upx + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
						upx = 2;
					}
				}
				{
					int ys = y_r(380+40);
					int ye = ys + y_r(30);
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						if (dnx == 1) {
							dnx = 2;
							if (hight_s > -camhigh) {
								hight_s-=8;
							}
							else {
								hight_s = -camhigh;
							}
						}
						dnx = std::min<size_t>(dnx + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
						dnx = 2;
					}
				}

				bottom_s = std::min(bottom_s, hight_s-8);
				//底面
				font40.DrawStringFormat(xs - x_r(400), y_r(480 + 15), GetColor(255, 255, 0), "設定する底面 : %d", bottom_s);
				{
					int ys = y_r(480);
					int ye = ys + y_r(30);
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						if (upy == 1) {
							upy = 2;
							if (bottom_s < camhigh) {
								bottom_s += 8;
								hight_s = std::max(bottom_s+8, hight_s);
							}
							else {
								bottom_s = camhigh;
							}
						}
						upy = std::min<size_t>(upy + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
						upy = 2;
					}
				}
				{
					int ys = y_r(480 + 40);
					int ye = ys + y_r(30);
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						if (dny == 1) {
							dny = 2;
							if (bottom_s > -camhigh) {
								bottom_s -= 8;
							}
							else {
								bottom_s = -camhigh;
							}
						}
						dny = std::min<size_t>(dny + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
						dny = 2;
					}
				}
			}
			//アンドゥ
			{
				int xs = int(x_size * dispy / std::max(x_size, y_size));
				int ys = y_r(580);
				int xe = xs + x_r(100);
				int ye = ys + y_r(40);
				char buf[] = "戻る";
				if (n_list.size() >= 2 && itr != n_list.begin()) {
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 255, 0));

						if (undo == 1) {
							itr--;
							n = *itr;
						}
						undo = std::min<size_t>(undo + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 0, 0));
						undo = 2;
					}
				}
				else {
					DrawBox(xs, ys, xe, ye, GetColor(128,128,128), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(0, 0, 0));
				}
			}
			//リドゥ
			{
				int xs = int(x_size * dispy / std::max(x_size, y_size)) + x_r(150);
				int ys = y_r(580);
				int xe = xs + x_r(100);
				int ye = ys + y_r(40);
				char buf[] = "進む";
				auto tr = itr;
				tr++;
				if (n_list.size() >= 2 && tr != n_list.end()) {
					if (inm(xs, ys, xe, ye)) {
						DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 255, 0));

						if (redo == 1) {
							itr++;
							n = *itr;
						}
						redo = std::min<size_t>(redo + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
					}
					else {
						DrawBox(xs, ys, xe, ye, GetColor(0, 255, 0), TRUE);
						font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 0, 0));
						redo = 2;
					}
				}
				else {
					DrawBox(xs, ys, xe, ye, GetColor(128, 128, 128), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(0, 0, 0));
				}
			}
			//終了
			{
				int xs = x_r(1920 - 340);
				int ys = y_r(1080 - 160);
				int xe = xs + x_r(300);
				int ye = ys + y_r(40);
				char buf[] = "保存せず終了";
				if (inm(xs, ys, xe, ye)) {
					DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 255, 0));
					if (cslcnt == 1) {
						cslcnt = 2;
						return false;
					}
					cslcnt = std::min<size_t>(cslcnt + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
				}
				else {
					DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 0, 0));
						cslcnt = 2;
				}
			}
			//終了
			{
				int xs = x_r(1920 - 340);
				int ys = y_r(1080 - 80);
				int xe = xs + x_r(300);
				int ye = ys + y_r(40);
				char buf[] = "保存して続行";

				if (inm(xs, ys, xe, ye)) {
					DrawBox(xs, ys, xe, ye, GetColor(255, 0, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 255, 0));
					if (mscnt == 1) {
						mscnt = 2;
						break;
					}
					mscnt = std::min<size_t>(mscnt + 1, ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0) ? 2 : 0);
				}
				else {
					DrawBox(xs, ys, xe, ye, GetColor(255, 255, 0), TRUE);
					font40.DrawString(xs + (xe - xs - font40.GetDrawWidth(buf)) / 2, ys, buf, GetColor(255, 0, 0));
						mscnt = 2;
				}
			}
			ScreenFlip();
		}
		n_list.clear();
		//mapデータ1書き込み(マップチップ)
		{
			file.open(("data/Map/" + mapname + "/1.dat").c_str(), std::ios::binary | std::ios::out);
			for (auto& m : n) {
				file.write((char*)&m, sizeof(m));
			}
			file.close();
			n.clear();
		}
		//mapデータ2書き込み(プレイヤー初期位置、使用テクスチャ指定)
		{
			file.open(("data/Map/" + mapname + "/2.dat").c_str(), std::ios::binary | std::ios::out);
			file.write((char*)&mapdata, sizeof(mapdata));
			file.close();
		}
		//mapデータ3書き込み(敵情報)
		{
			file.open(("data/Map/" + mapname + "/3.dat").c_str(), std::ios::binary | std::ios::out);
			for (auto& m : e)
				file.write((char*)&m, sizeof(m));
			file.close();
			e.clear();
		}
		return true;
	}
	//map選択
	inline void set_map(int *player_x, int *player_y, std::string mapname) {
		using namespace std::literals;
		std::fstream file;
		size_t map_x = 0, map_y = 0;
		//mapデータ1読み込み(マップチップ)
		{
			file.open(("data/Map/" + mapname + "/1.dat").c_str(), std::ios::binary | std::ios::in);
			do {
				ans.resize(ans.size() + 1);
				file.read((char*)&ans.back(), sizeof(ans.back()));
				map_x = std::max<size_t>(ans.back().xp, map_x);
				map_y = std::max<size_t>(ans.back().yp, map_y);
			} while (!file.eof());
			map_x++;
			map_y++;
			file.close();
			ans.pop_back();

			zcon.resize(map_x);
			for (auto& z : zcon) {
				z.resize(map_y);
			}
		}
		//mapデータ2読み込み(プレイヤー初期位置、使用テクスチャ指定)
		{
			maps mapb;
			file.open(("data/Map/" + mapname + "/2.dat").c_str(), std::ios::binary | std::ios::in);
			file.read((char*)&mapb, sizeof(mapb));
			file.close();
			*player_x = -mapb.plx;
			*player_y = -mapb.ply;
			light_yrad = mapb.light_yrad;
			GraphHandle::LoadDiv(mapb.wall_name, 32, 16, 2, 16, 32, &walls);
			GraphHandle::LoadDiv(mapb.floor_name, 256, 16, 16, 16, 16, &floors);
			set_human(&player_id, *player_x, *player_y);
		}
		//mapデータ3読み込み(敵情報)
		{
			file.open(("data/Map/" + mapname + "/3.dat").c_str(), std::ios::binary | std::ios::in);
			do {
				enemies anse;
				file.read((char*)&anse, sizeof(anse));
				size_t idb;
				set_human(&idb, anse.xp, anse.yp);
			} while (!file.eof());
			file.close();
		}
	}
	//mapの設置
	inline void put_map(void) {
		for (auto& z : ans) {
			set_drw_rect(z.xp, z.yp, z.bottom, z.hight, z.wall_id, z.floor_id, z.dir);
		}
	}
	//
	inline void exit_map(void) {
		walls.clear();
		floors.clear();
		human.clear();
		for (auto& z : zcon)
			z.clear();
		zcon.clear();
		return;
	}
	//壁描画
	inline void draw_wall(int UorL,con* z){//一辺
		const auto a00_1 = getpos(xpos + tilesize * (z->cpos.x + 0), ypos + tilesize * (z->cpos.y + 0), z->hight);
		const auto a10_1 = getpos(xpos + tilesize * (z->cpos.x + 1), ypos + tilesize * (z->cpos.y + 0), z->hight);
		const auto a01_1 = getpos(xpos + tilesize * (z->cpos.x + 0), ypos + tilesize * (z->cpos.y + 1), z->hight);
		const auto a11_1 = getpos(xpos + tilesize * (z->cpos.x + 1), ypos + tilesize * (z->cpos.y + 1), z->hight);

		const auto a00_0 = getpos(xpos + tilesize * (z->cpos.x + 0), ypos + tilesize * (z->cpos.y + 0), z->bottom);//◤
		const auto a10_0 = getpos(xpos + tilesize * (z->cpos.x + 1), ypos + tilesize * (z->cpos.y + 0), z->bottom);//◥
		const auto a01_0 = getpos(xpos + tilesize * (z->cpos.x + 0), ypos + tilesize * (z->cpos.y + 1), z->bottom);//◣
		const auto a11_0 = getpos(xpos + tilesize * (z->cpos.x + 1), ypos + tilesize * (z->cpos.y + 1), z->bottom);//◢
		if (UorL < 12) {
			if (UorL % 4 == 0) {
				int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(0))));
				SetDrawBright(c, c, c);
				switch (UorL) {
				case 0://縦(上)
					if (a00_0.y < a00_1.y && z->hight != z->bottom) {
						DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 4://上◢
					if (a10_0.y < a10_1.y && z->hight != z->bottom) {
						DrawModiGraph(a10_1.x, a10_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 8://上◣
					if (a00_0.y < a00_1.y && z->hight != z->bottom) {
						DrawModiGraph(a00_1.x, a00_1.y, a00_1.x, a00_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				}
			}
			if (UorL % 4 == 1) {
				int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(270))));
				SetDrawBright(c, c, c);
				switch (UorL) {
				case 1://横(左)
					if (a00_0.x < a00_1.x && z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a00_1.x, a00_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 5://左◢
					if (a00_0.x < a00_1.x && z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a01_1.x, a01_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 9://左◥
					if (a00_0.x < a00_1.x && z->hight != z->bottom) {
						DrawModiGraph(a00_1.x, a00_1.y, a00_1.x, a00_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				}
			}
			if (UorL % 4 == 2) {
				int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(180))));
				SetDrawBright(c, c, c);
				switch (UorL) {
				case 2://縦(下)
					if (a11_0.y >= a11_1.y && z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a11_1.x, a11_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 6://下◢
					if (a11_0.y >= a11_1.y && z->hight != z->bottom) {
						DrawModiGraph(a11_1.x, a11_1.y, a11_1.x, a11_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 10://下◣
					if (a11_0.y >= a11_1.y && z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a01_1.x, a01_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				}
			}
			if (UorL % 4 == 3) {
				int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(90))));
				SetDrawBright(c, c, c);
				switch (UorL) {
				case 3://横(右)
					if (a11_0.x >= a11_1.x && z->hight != z->bottom) {
						DrawModiGraph(a11_1.x, a11_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 7://右◢
					if (a11_0.x >= a11_1.x && z->hight != z->bottom) {
						DrawModiGraph(a11_1.x, a11_1.y, a11_1.x, a11_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 11://右◥
					if (a11_0.x >= a11_1.x && z->hight != z->bottom) {
						DrawModiGraph(a10_1.x, a10_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				}
			}
		}
		else if (UorL < 20) {
			if (z->hight != z->bottom) {
				float rad = std::clamp(sin(atan2f(float(z->hight - z->bottom), float(tilesize))), 0.5f, 1.f);
				//float rad = 1.f;
				if (UorL % 4 == 0) {

					int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(0)) *rad));
					SetDrawBright(c, c, c);
					switch (UorL) {
					case 12:
						if (a00_0.y < a01_1.y) {
							DrawModiGraph(a01_1.x, a01_1.y, a11_1.x, a11_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
						}
						break;
					case 16:
						if (a00_0.y < a01_1.y) {
							DrawModiGraph(a01_1.x, a01_1.y, a11_1.x, a11_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->floorhandle->get(), TRUE);
						}
						break;
					}
				}
				if (UorL % 4 == 1) {
					int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(270))*rad));
					SetDrawBright(c, c, c);
					switch (UorL) {
					case 13:
						if (a00_0.x < a11_1.x) {
							DrawModiGraph(a11_1.x, a11_1.y, a10_1.x, a10_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
						}
						break;
					case 17:
						if (a00_0.x < a11_1.x) {
							DrawModiGraph(a11_1.x, a11_1.y, a10_1.x, a10_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->floorhandle->get(), TRUE);
						}
						break;
					}
				}
				if (UorL % 4 == 2) {
					int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(180))*rad));
					SetDrawBright(c, c, c);
					switch (UorL) {
					case 14:
						if (a01_0.y > a10_1.y) {
							DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->floorhandle->get(), TRUE);
						}
						break;
					case 18:
						if (a01_0.y > a10_1.y) {
							DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
						}
						break;
					}
				}
				if (UorL % 4 == 3) {
					int c = int(128.0f*(1.f + cos(light_yrad + deg2rad(90))*rad));
					SetDrawBright(c, c, c);
					switch (UorL) {
					case 15:
						if (a10_0.x > a01_1.x) {
							DrawModiGraph(a01_1.x, a01_1.y, a00_1.x, a00_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->floorhandle->get(), TRUE);
						}
						break;
					case 19:
						if (a10_0.x > a01_1.x) {
							DrawModiGraph(a01_1.x, a01_1.y, a00_1.x, a00_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
						}
						break;
					}
				}
			}
			else {
				if (z->hight == 0) {
					SetDrawBright(255, 255, 255);
					DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_1.x, a11_1.y, a01_1.x, a01_1.y, z->floorhandle->get(), TRUE);
				}
				else if (z->hight >= camhigh) {
					SetDrawBright(0, 0, 0);
					DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_1.x, a11_1.y, a01_1.x, a01_1.y, z->floorhandle->get(), TRUE);
				}
				else {
					int c = 255 * (camhigh - z->hight) / camhigh;
					SetDrawBright(c, c, c);
					DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_1.x, a11_1.y, a01_1.x, a01_1.y, z->floorhandle->get(), TRUE);
				}
			}
		}
		else {
			if (z->hight == 0) {
				SetDrawBright(255, 255, 255);
				DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_1.x, a11_1.y, a01_1.x, a01_1.y, z->floorhandle->get(), TRUE);
			}
			else if (z->hight >= camhigh) {
				SetDrawBright(0,0,0);
				DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_1.x, a11_1.y, a01_1.x, a01_1.y, z->floorhandle->get(), TRUE);
			}
			else {
				int c = 255 * (camhigh - z->hight) / camhigh;
				SetDrawBright(c, c, c);
				DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_1.x, a11_1.y, a01_1.x, a01_1.y, z->floorhandle->get(), TRUE);
			}
		}
		SetDrawBright(255, 255, 255);
	}
	//柱描画
	inline void drw_rect(con* z){
		switch (z->use) {//三角柱
		case 0://上
			draw_wall(12, z);	//縦(上)//12
			draw_wall(5, z);	//横(左)
			draw_wall(2, z);	//縦(下)
			draw_wall(7, z);	//横(右)
			break;
		case 1://左
			draw_wall(4, z);	//縦(上)//4
			draw_wall(13, z);	//横(左)
			draw_wall(6, z);	//縦(下)
			draw_wall(3, z);	//横(右)
			break;
		case 2://下
			draw_wall(0, z);	//縦(上)//12
			draw_wall(9, z);	//横(左)
			draw_wall(18, z);	//縦(下)
			draw_wall(11, z);	//横(右)
			break;
		case 3://右
			draw_wall(8, z);	//縦(上)//8
			draw_wall(1, z);	//横(左)
			draw_wall(10, z);	//縦(下)
			draw_wall(19, z);	//横(右)
			break;


		case 4://上
			draw_wall(16, z);	//縦(上)//12
			draw_wall(5, z);	//横(左)
			draw_wall(2, z);	//縦(下)
			draw_wall(7, z);	//横(右)
			break;
		case 5://左
			draw_wall(4, z);	//縦(上)//4
			draw_wall(17, z);	//横(左)
			draw_wall(6, z);	//縦(下)
			draw_wall(3, z);	//横(右)
			break;
		case 6://下
			draw_wall(0, z);	//縦(上)//12
			draw_wall(9, z);	//横(左)
			draw_wall(14, z);	//縦(下)
			draw_wall(11, z);	//横(右)
			break;
		case 7://右
			draw_wall(8, z);	//縦(上)//8
			draw_wall(1, z);	//横(左)
			draw_wall(10, z);	//縦(下)
			draw_wall(15, z);	//横(右)
			break;


		default://柱
			draw_wall(0, z);	//縦(上)
			draw_wall(1, z);	//横(左)
			draw_wall(2, z);	//縦(下)
			draw_wall(3, z);	//横(右)
			draw_wall(20, z);	//天井
			break;
		}
	}
	//人描画
	inline void drw_human(con* z) {
		for (auto& pl : human) {
			for (auto& g : pl.sort) {
				auto q = getpos(pl.bone[g.first].xp + pl.pos.x, pl.bone[g.first].yp + pl.pos.y, z->bottom);
				if (q.x >= z->floor[0].x && q.y >= z->floor[0].y && q.x <= z->floor[3].x && q.y <= z->floor[3].y) {
					auto p = getpos(pl.bone[g.first].xp + pl.pos.x, pl.bone[g.first].yp + pl.pos.y, pl.bone[g.first].zp - pl.sort[0].second + z->hight);
					DrawRotaGraph3(p.x, p.y, 16, 16, 
						double(dispx) / 1920.0 * double( int64_t( int64_t(z->hight) + (pl.bone[g.first].zp) - (pl.sort[0].second) ) * 5  + camhigh ) / camhigh,
						double(dispx) / 1920.0 * double(int64_t(int64_t(z->hight) + (pl.bone[g.first].zp) - (pl.sort[0].second)) * 5 + camhigh) / camhigh,
						double(pl.bone[g.first].yrad) + double(pl.bone[g.first].yr), pl.Graphs[g.first].get(), TRUE);
				}
			}
		}
	}
	//壁影描画
	inline void draw_wall_shadow(int UorL, con* z) {//一辺
		const auto add_x = int(float(z->hight - z->bottom)*sin(light_yrad));
		const auto add_y = int(float(z->hight - z->bottom)*cos(light_yrad));
		const auto a00_1 = getpos(xpos + tilesize * (z->cpos.x + 0) + add_x, ypos + tilesize * (z->cpos.y + 0) + add_y, z->bottom);
		const auto a10_1 = getpos(xpos + tilesize * (z->cpos.x + 1) + add_x, ypos + tilesize * (z->cpos.y + 0) + add_y, z->bottom);
		const auto a01_1 = getpos(xpos + tilesize * (z->cpos.x + 0) + add_x, ypos + tilesize * (z->cpos.y + 1) + add_y, z->bottom);
		const auto a11_1 = getpos(xpos + tilesize * (z->cpos.x + 1) + add_x, ypos + tilesize * (z->cpos.y + 1) + add_y, z->bottom);
		const auto a00_0 = getpos(xpos + tilesize * (z->cpos.x + 0), ypos + tilesize * (z->cpos.y + 0), z->bottom);//◤
		const auto a10_0 = getpos(xpos + tilesize * (z->cpos.x + 1), ypos + tilesize * (z->cpos.y + 0), z->bottom);//◥
		const auto a01_0 = getpos(xpos + tilesize * (z->cpos.x + 0), ypos + tilesize * (z->cpos.y + 1), z->bottom);//◣
		const auto a11_0 = getpos(xpos + tilesize * (z->cpos.x + 1), ypos + tilesize * (z->cpos.y + 1), z->bottom);//◢

		SetDrawBright(0, 0, 0);
		if (UorL <= 20) {
			if (UorL % 4 == 0) {
				switch (UorL) {
				case 0://縦(上)
					if (z->hight != z->bottom) {
						DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 4:
					DrawModiGraph(a01_1.x, a01_1.y, a11_1.x, a11_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
					break;
				case 8://上◢
					if (z->hight != z->bottom) {
						DrawModiGraph(a10_1.x, a10_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 16://上◣
					if (z->hight != z->bottom) {
						DrawModiGraph(a00_1.x, a00_1.y, a00_1.x, a00_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 20:
					if (a00_0.y < a01_1.y) {
						DrawModiGraph(a01_1.x, a01_1.y, a11_1.x, a11_1.y, a10_0.x, a10_0.y, a00_0.x, a00_0.y, z->floorhandle->get(), TRUE);
					}
					break;
				}
			}
			if (UorL % 4 == 1) {
				switch (UorL) {
				case 1://横(左)
					if (z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a00_1.x, a00_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 5://左◢
					if (z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a01_1.x, a01_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 9:
						DrawModiGraph(a11_1.x, a11_1.y, a10_1.x, a10_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					break;
				case 13://左◥
					if (z->hight != z->bottom) {
						DrawModiGraph(a00_1.x, a00_1.y, a00_1.x, a00_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 17:
					if (a00_0.x < a11_1.x) {
						DrawModiGraph(a11_1.x, a11_1.y, a10_1.x, a10_1.y, a00_0.x, a00_0.y, a01_0.x, a01_0.y, z->floorhandle->get(), TRUE);
					}
					break;
				}
			}
			if (UorL % 4 == 2) {
				switch (UorL) {
				case 2://縦(下)
					if (z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a11_1.x, a11_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 6:
					if (a01_0.y > a10_1.y) {
						DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->floorhandle->get(), TRUE);
					}
					break;
				case 10://下◢
					if (z->hight != z->bottom) {
						DrawModiGraph(a11_1.x, a11_1.y, a11_1.x, a11_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 14:
						DrawModiGraph(a00_1.x, a00_1.y, a10_1.x, a10_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					break;
				case 18://下◣
					if (z->hight != z->bottom) {
						DrawModiGraph(a01_1.x, a01_1.y, a01_1.x, a01_1.y, a11_0.x, a11_0.y, a01_0.x, a01_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				}
			}
			if (UorL % 4 == 3) {
				switch (UorL) {
				case 3://横(右)
					if (z->hight != z->bottom) {
						DrawModiGraph(a11_1.x, a11_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 7://右◢
					if (z->hight != z->bottom) {
						DrawModiGraph(a11_1.x, a11_1.y, a11_1.x, a11_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 11:
					if (a10_0.x > a01_1.x) {
						DrawModiGraph(a01_1.x, a01_1.y, a00_1.x, a00_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->floorhandle->get(), TRUE);
					}
					break;
				case 15://右◥
					if (z->hight != z->bottom) {
						DrawModiGraph(a10_1.x, a10_1.y, a10_1.x, a10_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
					}
					break;
				case 19:
						DrawModiGraph(a01_1.x, a01_1.y, a00_1.x, a00_1.y, a10_0.x, a10_0.y, a11_0.x, a11_0.y, z->wallhandle->get(), TRUE);
					break;
				}
			}
		}
		SetDrawBright(255, 255, 255);
	}
	//柱の影描画
	inline void drw_rect_shadow(con* z) {
		switch (z->use) {//三角柱
		case 0://上
			draw_wall_shadow(4, z);		//縦(上)//4
			draw_wall_shadow(5, z);		//横(左)
			draw_wall_shadow(2, z);		//縦(下)
			draw_wall_shadow(7, z);		//横(右)
			break;
		case 1://左
			draw_wall_shadow(8, z);		//縦(上)//8
			draw_wall_shadow(9, z);		//横(左)
			draw_wall_shadow(10, z);	//縦(下)
			draw_wall_shadow(3, z);		//横(右)
			break;
		case 2://下
			draw_wall_shadow(0, z);		//縦(上)//12
			draw_wall_shadow(13, z);	//横(左)
			draw_wall_shadow(14, z);	//縦(下)
			draw_wall_shadow(15, z);	//横(右)
			break;
		case 3://右
			draw_wall_shadow(16, z);	//縦(上)//16
			draw_wall_shadow(1, z);		//横(左)
			draw_wall_shadow(18, z);	//縦(下)
			draw_wall_shadow(19, z);	//横(右)
			break;

		case 4://上
			draw_wall_shadow(20, z);	//縦(上)//4
			draw_wall_shadow(5, z);		//横(左)
			draw_wall_shadow(2, z);		//縦(下)
			draw_wall_shadow(7, z);		//横(右)
			break;
		case 5://左
			draw_wall_shadow(8, z);		//縦(上)//8
			draw_wall_shadow(17, z);	//横(左)
			draw_wall_shadow(10, z);	//縦(下)
			draw_wall_shadow(3, z);		//横(右)
			break;
		case 6://下
			draw_wall_shadow(0, z);		//縦(上)//12
			draw_wall_shadow(13, z);	//横(左)
			draw_wall_shadow(6, z);		//縦(下)
			draw_wall_shadow(15, z);	//横(右)
			break;
		case 7://右
			draw_wall_shadow(16, z);	//縦(上)//16
			draw_wall_shadow(1, z);		//横(左)
			draw_wall_shadow(18, z);	//縦(下)
			draw_wall_shadow(11, z);	//横(右)
			break;

		default://柱
			draw_wall_shadow(0, z);		//縦(上)
			draw_wall_shadow(1, z);		//横(左)
			draw_wall_shadow(2, z);		//縦(下)
			draw_wall_shadow(3, z);		//横(右)
			break;
		}
	}
	//人の影描画
	inline void drw_human_shadow(con* z) {
		SetDrawBright(0,0,0);
		for (auto& pl : human) {
			for (auto& g : pl.sort) {
				auto q = getpos(pl.bone[g.first].xp + pl.pos.x, pl.bone[g.first].yp + pl.pos.y, z->bottom);
				if (q.x >= z->floor[0].x && q.y >= z->floor[0].y && q.x <= z->floor[3].x && q.y <= z->floor[3].y) {
					auto p = getpos(
						pl.bone[g.first].xp + pl.pos.x + int(float(pl.bone[g.first].zp - pl.sort[0].second)*sin(light_yrad)),
						pl.bone[g.first].yp + pl.pos.y + int(float(pl.bone[g.first].zp - pl.sort[0].second)*cos(light_yrad)),
						z->hight);
					DrawRotaGraph3(p.x, p.y, 16, 16,
						double(dispx) / 1920.0 * double(int64_t(int64_t(z->hight) + (pl.bone[g.first].zp) - (pl.sort[0].second)) * 5 + camhigh) / camhigh,
						double(dispx) / 1920.0 * double(int64_t(int64_t(z->hight) + (pl.bone[g.first].zp) - (pl.sort[0].second)) * 5 + camhigh) / camhigh,
						double(pl.bone[g.first].yrad) + double(pl.bone[g.first].yr), pl.Graphs[g.first].get(), TRUE);

				}
			}
		}
		SetDrawBright(255, 255, 255);
	}
	//カメラ座標設定
	inline void set_cam(int sx, int sy) {
		xpos = sx;
		ypos = sy;
		put_map();
		return;
	}
	//人のデータ作成
	inline void set_human(size_t* id,int xp,int yp) {
		using namespace std::literals;
		*id = human.size();
		human.resize(*id + 1);
		auto& z = human[*id];
		GraphHandle::LoadDiv("data/Char/1.bmp", 33, 11, 3, 32, 32, &z.Graphs);
		z.sort.resize(z.Graphs.size());
		/*
		{//キャラバイナリ書き込み
			std::vector<Bonesdata> n;
			n.clear();
			{
				{//左腕
					n.resize(n.size() + 1);
					n.back().parent = 1;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -2.0f;

					n.resize(n.size() + 1);
					n.back().parent = 2;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -5.0f;

					n.resize(n.size() + 1);
					n.back().parent = 3;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -4.0f;

					n.resize(n.size() + 1);
					n.back().parent = 4;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -5.0f;

					n.resize(n.size() + 1);
					n.back().parent = 5;
					n.back().xdist = -12.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = 0.0f;
				}
				n.resize(n.size() + 1);
				n.back().parent = 15;
				n.back().xdist = 0.0f;
				n.back().ydist = 0.0f;
				n.back().zdist = -3.0f;
				{//右腕
					n.resize(n.size() + 1);
					n.back().parent = 5;
					n.back().xdist = 12.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = 0.0f;

					n.resize(n.size() + 1);
					n.back().parent = 6;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -5.0f;

					n.resize(n.size() + 1);
					n.back().parent = 7;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -4.0f;

					n.resize(n.size() + 1);
					n.back().parent = 8;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -5.0f;

					n.resize(n.size() + 1);
					n.back().parent = 9;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -2.0f;
				}
			}
			n.resize(n.size() + 1);
			n.resize(n.size() + 1);
			n.resize(n.size() + 1);
			n.resize(n.size() + 1);

			n.resize(n.size() + 1);
			n.back().parent = -1;
			n.back().xdist = 0.0f;
			n.back().ydist = 0.0f;
			n.back().zdist = 0.0f;

			n.resize(n.size() + 1);
			n.back().parent = 5;
			n.back().xdist = 0.0f;
			n.back().ydist = 0.0f;
			n.back().zdist = -3.0f;

			n.resize(n.size() + 1);
			n.resize(n.size() + 1);
			n.resize(n.size() + 1);
			n.resize(n.size() + 1);
			n.resize(n.size() + 1);

			{
				{
					n.resize(n.size() + 1);
					n.back().parent = 23;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -1.0f;

					n.resize(n.size() + 1);
					n.back().parent = 24;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -4.0f;

					n.resize(n.size() + 1);
					n.back().parent = 25;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -3.0f;

					n.resize(n.size() + 1);
					n.back().parent = 26;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -4.0f;

					n.resize(n.size() + 1);
					n.back().parent = 27;
					n.back().xdist = -5.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -3.0f;
					//n.back().yrad = deg2rad(-90);
					//n.back().xrad = deg2rad(-70);
				}
				n.resize(n.size() + 1);
				n.back().parent = 16;
				n.back().xdist = 0.0f;
				n.back().ydist = 0.0f;
				n.back().zdist = -3.0f;
				{
					n.resize(n.size() + 1);
					n.back().parent = 27;
					n.back().xdist = 5.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -3.0f;
					//n.back().yrad = deg2rad(-90);
					//n.back().xrad = deg2rad(-70);

					n.resize(n.size() + 1);
					n.back().parent = 28;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -4.0f;

					n.resize(n.size() + 1);
					n.back().parent = 29;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -3.0f;

					n.resize(n.size() + 1);
					n.back().parent = 30;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -4.0f;

					n.resize(n.size() + 1);
					n.back().parent = 31;
					n.back().xdist = 0.0f;
					n.back().ydist = 0.0f;
					n.back().zdist = -1.0f;
				}
			}

			std::fstream file;
			// 書き込む
			file.open("data/Char/1.dat", std::ios::binary | std::ios::out);
			for (auto& m : n) {
				file.write((char*)&m, sizeof(m));
			}
			file.close();
		}
		//*/
		{//モーションテキスト(直に打ち込めるように)
			for (int i = 0; i < 2; i++) {
				z.anime.resize(z.anime.size() + 1);
				const auto mdata = FileRead_open(("data/Char/Mot/"s+std::to_string(i)+".mot").c_str(), FALSE);
				{
					do {
						int tmp;
						std::string ttt = getcmd(mdata, &tmp);
						if (ttt.find("frame") != std::string::npos) {
							z.anime.back().resize(z.anime.back().size() + 1);
							z.anime.back().back().bone.resize(33);//todo
							z.anime.back().back().time = tmp;
						}
						else if (ttt.find("left_arm_") != std::string::npos) {
							if (ttt.find("x") != std::string::npos) {
								z.anime.back().back().bone[4].xrad = deg2rad(tmp);
							}
							else if (ttt.find("y") != std::string::npos) {
								z.anime.back().back().bone[4].yrad = deg2rad(tmp);
							}
							else if (ttt.find("z") != std::string::npos) {
								z.anime.back().back().bone[4].zrad = deg2rad(tmp);
							}
						}
						else if (ttt.find("right_arm_") != std::string::npos) {
							if (ttt.find("x") != std::string::npos) {
								z.anime.back().back().bone[6].xrad = deg2rad(tmp);
							}
							else if (ttt.find("y") != std::string::npos) {
								z.anime.back().back().bone[6].yrad = deg2rad(tmp);
							}
							else if (ttt.find("z") != std::string::npos) {
								z.anime.back().back().bone[6].zrad = deg2rad(tmp);
							}
						}

						else if (ttt.find("left_leg_") != std::string::npos) {
							if (ttt.find("x") != std::string::npos) {
								z.anime.back().back().bone[26].xrad = deg2rad(tmp);
							}
							else if (ttt.find("y") != std::string::npos) {
								z.anime.back().back().bone[26].yrad = deg2rad(tmp);
							}
							else if (ttt.find("z") != std::string::npos) {
								z.anime.back().back().bone[26].zrad = deg2rad(tmp);
							}
						}
						else if (ttt.find("right_leg_") != std::string::npos) {
							if (ttt.find("x") != std::string::npos) {
								z.anime.back().back().bone[28].xrad = deg2rad(tmp);
							}
							else if (ttt.find("y") != std::string::npos) {
								z.anime.back().back().bone[28].yrad = deg2rad(tmp);
							}
							else if (ttt.find("z") != std::string::npos) {
								z.anime.back().back().bone[28].zrad = deg2rad(tmp);
							}
						}


						else if (ttt.find("end") != std::string::npos) {
							break;
						}
					} while (true);
				}
				FileRead_close(mdata);
			}
		}
		{//キャラバイナリ
			std::fstream file;
			file.open("data/Char/1.dat", std::ios::binary | std::ios::in);
			do {
				z.bone.resize(z.bone.size() + 1);
				file.read((char*)&z.bone.back(), sizeof(z.bone.back()));
			} while (!file.eof());
			z.bone.pop_back();
			file.close();
		}
		z.pos.x = xp;
		z.pos.y = yp;
	}
	//人と壁の判定
	inline void hit_wall(pos2D *pos, pos2D& old) {
		pos->x = std::clamp(pos->x, tilesize / 4, tilesize * int(zcon.size())- tilesize / 4);
		pos->y = std::clamp(pos->y, tilesize / 4, tilesize * int(zcon.back().size())- tilesize / 4);

		for (auto& c : zcon) {
			for (auto& z : c) {
				if (z.hight != z.bottom) {
					const auto x0 = tilesize * z.cpos.x - tilesize / 4;
					const auto y0 = tilesize * z.cpos.y - tilesize / 4;
					const auto x1 = tilesize * z.cpos.x + tilesize * 5 / 4;
					const auto y1 = tilesize * z.cpos.y + tilesize * 5 / 4;
					//*
					auto a = *pos - old;
					{
						pos2D s = { x0,y0 };
						pos2D p = { x0 - s.x ,y1 - s.y };
						if (ColSeg(old, a, s, p)) {
							auto pr = p.hydist();
							pos->x -= -p.y * p.cross(&a) / pr;
							pos->y -= p.x * p.cross(&a) / pr;
							break;
						}
					}
					{
						pos2D s = { x1,y0 };
						pos2D p = { x0 - s.x ,y0 - s.y };
						if (ColSeg(old, a, s, p)) {
							auto pr = p.hydist();
							pos->x -= -p.y * p.cross(&a) / pr;
							pos->y -= p.x * p.cross(&a) / pr;
							break;
						}
					}
					{
						pos2D s = { x1 ,y1 };
						pos2D p = { x1 - s.x ,y0 - s.y };
						if (ColSeg(old, a, s, p)) {
							auto pr = p.hydist();
							pos->x -= -p.y * p.cross(&a) / pr;
							pos->y -= p.x * p.cross(&a) / pr;
							break;
						}
					}
					{
						pos2D s = { x0,y1 };
						pos2D p = { x1 - s.x ,y1 - s.y };
						if (ColSeg(old, a, s, p)) {
							auto pr = p.hydist();
							pos->x -= -p.y * p.cross(&a) / pr;
							pos->y -= p.x * p.cross(&a) / pr;
							break;
						}
					}
					//*/
				}
			}
		}
	}
	//人の移動処理
	inline void move_human(int *x_pos, int *y_pos) {
		for (int i = 0; i < human.size(); i++) {
			human[i].vtmp = human[i].pos;
			if (i != player_id) {
				//todo : cpu
				//human[i].pos.x = 100*i;
				//human[i].pos.y = 100*i;
			}
			else{
				//自機の移動
				human[player_id].pos.x = -*x_pos;
				human[player_id].pos.y = -*y_pos;
			}
			hit_wall(&human[i].pos, human[i].vtmp);
			if (i == player_id) {
				*x_pos = -human[player_id].pos.x;
				*y_pos = -human[player_id].pos.y;
			}
			human[i].vtmp -= human[i].pos;
		}
	}
	//柱設置
	inline void set_drw_rect(size_t px, size_t py, int bottom, int hight, int walldir, int floordir, int UorL = -1) {//三角柱
		auto& z = zcon[px][py];
		z.cpos.x = int(px);
		z.cpos.y = int(py);
		z.bottom = bottom;
		z.hight = hight;

		z.top[0] = getpos(xpos + tilesize * (z.cpos.x + 0), ypos + tilesize * (z.cpos.y + 0), z.hight);
		z.top[1] = getpos(xpos + tilesize * (z.cpos.x + 1), ypos + tilesize * (z.cpos.y + 0), z.hight);
		z.top[2] = getpos(xpos + tilesize * (z.cpos.x + 0), ypos + tilesize * (z.cpos.y + 1), z.hight);
		z.top[3] = getpos(xpos + tilesize * (z.cpos.x + 1), ypos + tilesize * (z.cpos.y + 1), z.hight);

		z.floor[0] = getpos(xpos + tilesize * (z.cpos.x + 0), ypos + tilesize * (z.cpos.y + 0), z.bottom);
		z.floor[1] = getpos(xpos + tilesize * (z.cpos.x + 1), ypos + tilesize * (z.cpos.y + 0), z.bottom);
		z.floor[2] = getpos(xpos + tilesize * (z.cpos.x + 0), ypos + tilesize * (z.cpos.y + 1), z.bottom);
		z.floor[3] = getpos(xpos + tilesize * (z.cpos.x + 1), ypos + tilesize * (z.cpos.y + 1), z.bottom);

		z.wallhandle = &walls[walldir];
		z.floorhandle = &floors[floordir];
		z.use = UorL;
		//z.use = std::clamp(UorL, -1, 4);
	}
	//人の描画用意
	inline void ready_player(void) {
		for (auto& pl : human) {
			for (auto& g : pl.bone) {
				g.edit = false;
			}

			//ここでアニメーション
			{
				auto& x = pl.anime[pl.animesel];
				auto& a = x[pl.animeframe];
				auto& b = x[(pl.animeframe + 1) % int(x.size())];
				if (pl.animetime < a.time) {
					for (int z = 0; z < pl.bone.size(); z++) {
						pl.bone[z].xrad = a.bone[z].xrad + (b.bone[z].xrad - a.bone[z].xrad)*pl.animetime / a.time;
						pl.bone[z].yrad = a.bone[z].yrad + (b.bone[z].yrad - a.bone[z].yrad)*pl.animetime / a.time;
						pl.bone[z].zrad = a.bone[z].zrad + (b.bone[z].zrad - a.bone[z].zrad)*pl.animetime / a.time;
					}
					pl.animetime++;
				}
				else {
					pl.animeframe = (pl.animeframe + 1) % int(x.size());
					pl.animetime = 0;
				}
			}
			auto o = pl.animesel;

			if (pl.vtmp.x == 0 && pl.vtmp.y == 0) {
				pl.animesel = 1;
			}
			else {
				pl.animesel = 0;
			}

			if (o != pl.animesel) {
				pl.animeframe = 0;
				pl.animetime = 0;
			}
			//
			if (pl.vtmp.x != 0 || pl.vtmp.y != 0) {
				pl.vec = pl.vtmp;
			}
			{
				//移動方向に向く
				auto b = sqrtf(float(pl.vec.hydist()));
				auto q = (float(pl.vec.x)*cos(pl.yrad) - float(pl.vec.y)* -sin(pl.yrad)) / b;
				if (q > sin(deg2rad(10))) {
					pl.yrad += deg2rad(-5);
				}else if (q < sin(deg2rad(-10))) {
					pl.yrad += deg2rad(5);
				}
				//真後ろ振り向き
				if ((float(pl.vec.x)* -sin(pl.yrad) + float(pl.vec.y)*cos(pl.yrad)) / b <= -0.5) {
					pl.yrad += deg2rad(10);
				}
			}
			//座標指定
			bool next = false;
			do {
				next = false;
				for (auto& z : pl.bone) {
					auto p = z.parent;
					if (!z.edit) {
						if (p == -1) {
							z.xp = xpos;
							z.yp = ypos;
							z.zp = 0;
							z.xr = z.xrad;
							z.yr = z.yrad + pl.yrad;
							z.zr = z.zrad;
							z.edit = true;
						}
						else {
							if (pl.bone[p].edit) {
								z.xr = pl.bone[p].xrad + pl.bone[p].xr;
								z.yr = pl.bone[p].yrad + pl.bone[p].yr;
								z.zr = pl.bone[p].yrad + pl.bone[p].zr;

								float y1 = cos(z.xr)*z.ydist + sin(z.xr)*z.zdist;
								float z1 = cos(z.xr)*z.zdist - sin(z.xr)*z.ydist;
								float x2 = cos(z.zr)*z.xdist + sin(z.zr)*z1;

								z.xp = pl.bone[p].xp + int(cos(z.yr)*x2 - sin(z.yr)*y1);
								z.yp = pl.bone[p].yp + int(cos(z.yr)*y1 + sin(z.yr)*x2);
								z.zp = pl.bone[p].zp + int(cos(z.zr)*z1 - sin(z.zr)*z.xdist);
								z.edit = true;
							}
							else {
								next = true;
							}
						}
					}
				}
			} while (next);

			//高さでソート
			for (int i = 0; i < pl.sort.size(); i++) {
				pl.sort[i].first = i;
				pl.sort[i].second = pl.bone[i].zp;
			}
			for (int i = 0; i < pl.sort.size(); i++) {
				for (int j = i; j < pl.sort.size(); j++) {
					if (pl.sort[j].second <= pl.sort[i].second) {
						std::swap(pl.sort[j], pl.sort[i]);
					}
				}
			}
		}
	}
	//一気に描画
	inline void put_drw(void) {
		ready_player();

		const auto limmin = getpos(-dispx * 3 / 4, -dispy * 3 / 4, 0);
		const auto cam = getpos(0, 0, 0);
		const auto limmax = getpos(dispx * 3 / 4, dispy * 3 / 4, 0);

		//light_yrad += deg2rad(1);
		//DRAW
		SetDrawScreen(screen);
		ClearDrawScreen();
		//地面
		for (auto& c : zcon) {
			for (auto& z : c) {
				if (z.top[0].x <= cam.x && z.top[0].y <= cam.y && z.floor[3].y >= limmin.y  && z.floor[3].x >= limmin.x && z.bottom == z.hight) {
					drw_rect(&z);
				}
			}
			for (int y = int(c.size()) - 1; y >= 0; --y) {
				auto& z = c[y];
				if (z.top[0].x <= cam.x && z.top[3].y >= cam.y && z.floor[0].y <= limmax.y  && z.floor[3].x >= limmin.x && z.bottom == z.hight) {
					drw_rect(&z);
				}
			}
		}
		for (int x = int(zcon.size()) - 1; x >= 0; --x) {
			for (auto& z : zcon[x]) {
				if (z.top[3].x >= cam.x && z.top[0].y <= cam.y && z.floor[3].y >= limmin.y && z.floor[0].x <= limmax.x && z.bottom == z.hight) {
					drw_rect(&z);
				}
			}
			for (int y = int(zcon[x].size()) - 1; y >= 0; --y) {
				auto& z = zcon[x][y];
				if (z.top[3].x >= cam.x && z.top[3].y >= cam.y&& z.floor[0].y <= limmax.y  && z.floor[0].x <= limmax.x && z.bottom == z.hight) {
					drw_rect(&z);
				}

			}
		}
		//影
		//*
		SetDrawScreen(shadow_graph);
		ClearDrawScreen();
		for (auto& c : zcon) {
			for (auto& z : c) {
				if (z.floor[0].y <= limmax.y  && z.floor[0].x <= limmax.x && z.floor[3].y >= limmin.y && z.floor[3].x >= limmin.x) {
					if (z.bottom != z.hight) {
						drw_rect_shadow(&z);
					}
					drw_human_shadow(&z);
				}
			}
		}
		SetDrawScreen(screen);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
		DrawGraph(0, 0, shadow_graph, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		//*/
		//上階
		for (auto& c : zcon) {
			for (auto& z : c) {
				if (z.top[0].x <= cam.x && z.top[0].y <= cam.y && z.floor[3].y >= limmin.y && z.floor[3].x >= limmin.x) {
					if (z.bottom != z.hight) {
						drw_rect(&z);
					}
					drw_human(&z);
				}
			}
			for (int y = int(c.size()) - 1; y >= 0; --y) {
				auto& z = c[y];
				if (z.top[0].x <= cam.x && z.top[3].y >= cam.y && z.floor[0].y <= limmax.y  && z.floor[3].x >= limmin.x) {
					if (z.bottom != z.hight) {
						drw_rect(&z);
					}
					drw_human(&z);
				}
			}
		}
		for (int x = int(zcon.size()) - 1; x >= 0; --x) {
			for (auto& z : zcon[x]) {
				if (z.top[3].x >= cam.x && z.top[0].y <= cam.y && z.floor[3].y >= limmin.y && z.floor[0].x <= limmax.x) {
					if (z.bottom != z.hight) {
						drw_rect(&z);
					}
					drw_human(&z);
				}
			}
			for (int y = int(zcon[x].size()) - 1; y >= 0; --y) {
				auto& z = zcon[x][y];
				if (z.top[3].x >= cam.x && z.top[3].y >= cam.y&& z.floor[0].y <= limmax.y  && z.floor[0].x <= limmax.x){
					if (z.bottom != z.hight) {
						drw_rect(&z);
					}
					drw_human(&z);
				}
			}
		}

		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();
		return;
	}
	//出力
	inline void out_draw(void) {
		DrawGraph(0, 0, screen, TRUE);
	}
};

//デバッグ描画
class DeBuG {
private:
	float deb[60][6 + 1] = { 0.f };
	LONGLONG waypoint=0;
	float waydeb[6] = {0.f};
	size_t seldeb=0;
	FontHandle font;
public:
	DeBuG(void);
	void put_way(void);
	void end_way(void);
	void debug(void);
};

#endif
