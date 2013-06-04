#pragma once
class Effects
{
	enum FX{EQ, DIST, };
	DWORD record;
public:
	Effects(DWORD record);
	~Effects(void);
};

