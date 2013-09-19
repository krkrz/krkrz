/* 'turn' トランジション用変形変換テーブル */
/* このファイルは mkturntranstable.pl により作成されました */

#ifndef turntrans_table_H
#define turntrans_table_H

#pragma pack(push, 4)
struct tTurnTransParams
{
	int start;
	int len;
	int sx; // source start x ( * 65536 )
	int sy; // source start y ( * 65536 )
	int ex; // source end x ( * 65536 )
	int ey; // source end y ( * 65536 )
	int stepx; // source step x ( * 65536 )
	int stepy; // source step y ( * 65536 )
};
#pragma pack(pop)

extern const tTurnTransParams TurnTransParams[64][64];


#endif

