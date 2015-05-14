

#ifndef __MOUSE_CURSOR_H__
#define __MOUSE_CURSOR_H__

#include <vector>

class MouseCursor {
	enum {
		CURSOR_APPSTARTING,	// �W�����J�[�\������я��^�����v�J�[�\��
		CURSOR_ARROW,		// �W�����J�[�\��
		CURSOR_CROSS,		// �\���J�[�\��
		CURSOR_HAND,		// �n���h�J�[�\��
		CURSOR_IBEAM,		// �A�C�r�[�� (�c��) �J�[�\��
		CURSOR_HELP,		// ���Ƌ^�╄
		CURSOR_NO,			// �֎~�J�[�\��
		CURSOR_SIZEALL,		// 4 �������J�[�\��
		CURSOR_SIZENESW,	// �΂ߍ�������̗��������J�[�\��
		CURSOR_SIZENS,		// �㉺���������J�[�\��
		CURSOR_SIZENWSE,	// �΂߉E������̗��������J�[�\��
		CURSOR_SIZEWE,		// ���E���������J�[�\��
		CURSOR_UPARROW,		// �����̖��J�[�\��
		CURSOR_WAIT,		// �����v�J�[�\�� 
		CURSOR_EOT,
	};
	static const int CURSOR_OFFSET = 22;
	static const int CURSOR_INDEXES_NUM = 24;
	static const int CURSOR_INDEXES[CURSOR_INDEXES_NUM]; // �����̃J�[�\���C���f�b�N�X�ƌ��J�J�[�\���C���f�b�N�X�̕ϊ��e�[�u��
	static std::vector<HCURSOR> CURSOR_HANDLES_FOR_INDEXES;	// �S�J�[�\���̃n���h���A�V�K�Ǎ��݂��ꂽ���̂͒ǉ������

	static const LPTSTR CURSORS[CURSOR_EOT];	// �J�[�\���ƃ��\�[�XID�̑Ή��e�[�u��
	static HCURSOR CURSOR_HANDLES[CURSOR_EOT];	// �f�t�H���g�J�[�\���̃n���h���e�[�u��
	static const int INVALID_CURSOR_INDEX = 0x7FFFFFFF;	// �����ȃJ�[�\���C���f�b�N�X
	static bool CURSOR_INITIALIZED;	// �J�[�\���������ς݂��ۂ�

	static bool is_cursor_hide_;	// �J�[�\����crNone�Ŕ�\���ɂȂ��Ă��邩�ǂ���

public:
	static void Initialize();
	static void Finalize();
	static void SetMouseCursor( int index );
	static int AddCursor( HCURSOR hCursor ) {
		int index = CURSOR_HANDLES_FOR_INDEXES.size();
		CURSOR_HANDLES_FOR_INDEXES.push_back( hCursor );
		return index - CURSOR_OFFSET;
	}

private:
	int cursor_index_;

	int GetCurrentCursor();

public:
	MouseCursor() : cursor_index_(INVALID_CURSOR_INDEX) {}
	MouseCursor( int index ) : cursor_index_(index) {}

	void UpdateCursor();

	void SetCursorIndex( int index, HWND hWnd );
};

#endif
