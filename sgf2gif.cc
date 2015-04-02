
#include "pch.h"

// #define RENJU_ONLY

using namespace Gdiplus;
using namespace std;

const int MARGIN = 4; // �߾�, ��ֹ���ݽ�����ͼƬ�߽�
const int TITLE_HEIGHT = 20; 
const int LEFT_SPACE_WIDTH = 0;

const int MAX_BOARD_SIZE = 27; 
const int MAX_MOVE = MAX_BOARD_SIZE * MAX_BOARD_SIZE * 4; // ugly, but works

const int BOARD_LEFT = LEFT_SPACE_WIDTH + MARGIN;
const int BOARD_TOP = TITLE_HEIGHT + MARGIN;

// ѡ��Ի����������, �ʼ��ʱ��ֻ��Ǽ򵥵���Ϣ, ����Ҫ�ǵö�������, �Ѿ��е㲻����.
struct Options
{
	int delay; // �ӳ�, �������ٶ�
	int numbers; // ���Ķ���������ʾ����
	bool splitPeriodically; // �Ƿ�ָ�
	int splitCount; // �ָ��ĸ���
	int splitPoints[20]; // �ָ��
	int cw; // cell width, ���ӳߴ�

	// �Ƿ����Ҫ�ָ�ͼƬ
	bool RealSplit()
	{
		return splitPeriodically || splitCount > 0;
	}

	// �ӳ�תΪ�ַ���
	string GetDelayString()
	{
		char buf[32];
		sprintf(buf, "%d", delay);
		return buf;
	}

	void SetDelayString(const string & s)
	{
		delay = 50;
		delay = atoi(s.c_str());
		if(delay <0)
		{
			delay = 0;
		}
	}

	string GetNumbersString()
	{
		char buf[32];
		sprintf(buf, "%d", numbers);
		return buf;
	}

	void SetNumbersString(const string & s)
	{
		numbers = 1;
		numbers = atoi(s.c_str());
	}

	string GetSplitString()
	{
		if(splitPeriodically)
		{
			char buf[32];
			sprintf(buf, "*%d", splitPoints[0]);
			return buf;
		}
		else
		{
			string ret;
			for(int i=0; i<splitCount; i++)
			{
				char buf[32];
				sprintf(buf, "%d ", splitPoints[i]);
				ret+= buf;
			}
			if(ret.length() > 0)
			{
				ret.pop_back();
			}
			return ret;
		}
	}

	void SetSplitString(const string & s)
	{
		if(s.empty())
		{
			splitPeriodically = false;
			splitCount = 0;
			return;
		}
		if(s[0] == '*')
		{
			splitPeriodically = true;
			splitCount = 1;
			splitPoints[0] = 50;
			if(s.length() > 1)
			{
				splitPoints[0] = atoi(&s[1]);
				if(splitPoints[0] < 5)
				{
					splitPoints[0] = 5;
				}
			}
		}
		else
		{
			splitPeriodically = false;
			splitCount = 0;
			string tmp;
			int n = 0;
			for(int i=0; i<s.length() && splitCount<20;)
			{
				tmp.clear();
				while(i<s.length() && (s[i] <= 32 || s[i] ==',' || s[i] == '|'))
				{
					i++;
				}
				while(i<s.length() && s[i] > 32 && s[i] != ',' && s[i] !='|' )
				{
					tmp.push_back(s[i]);
					i++;
				}
				int n1 = atoi(tmp.c_str());
				if(n1 > n + 5)
				{
					n = n1;
					splitPoints[splitCount] = n;
					splitCount++;
				}
			}
		}
	}

	string GetCWString()
	{
		char buf[32];
		sprintf(buf, "%d", cw);
		return buf;
	}

	void SetCWString(const string & s)
	{
		cw = 23;
		cw = atoi(s.c_str());

		if(cw < 15)
		{
			cw = 15;
		}
		else if(cw > 50)
		{
			cw = 50;
		}
	}

} g_options;


int CW() // cell width (pixels)
{
	return g_options.cw;
}

int BW(int BS) // board width (pixels)
{
	return BOARD_LEFT + CW() * BS + MARGIN + 16;
}
int BH(int BS) // board height (pixels)
{
	return BOARD_TOP + CW() * BS + MARGIN + 16;
}

enum StoneColor
{
	None = 0,
	Black = 1,
	White = 2,
};

// �����ϵĽ����
struct Cross
{
	char x;
	char y;
};

struct Move : public Cross 
{
	bool addition; // �Ƿ�����
	StoneColor color;
	Move()
	{
		color = None;
		addition = false;
		x = 0;
		y = 0;
	}
};

struct Game
{
	// ��������, �μ�SFG�����Ķ���, GM[x]. ����ֻ֧���������Χ��
	enum GameType
	{
		gameUnkown = 0,
		gameGo = 1,
		gameRenju = 4
		
	}gameType;

	int boardSize;
	StoneColor winner;
	bool winByResign; // ����ʤ, +R
	
	string komi; // ��Ŀ, �ô�����
	string result; // ���Ľ��, �ı�
	
	string whiteName;
	string whiteRank;

	string blackName;
	string blackRank;

	int moveCount; // ��ǰ�Ĳ���
	Move moves[MAX_MOVE]; 

	Game()
	{
		winner = None;
		winByResign = false;
		moveCount = 0;
		boardSize = 19;
#ifdef RENJU_ONLY
		gameType = gameRenju;
#else
		gameType = gameGo;
#endif
	}
	
	string GetTitle()
	{
		string ret;
		if(!whiteName.empty() || !blackName.empty())
		{
			ret +=  whiteName.empty() ? "��: δ֪" : "��: " +whiteName;
			ret += whiteRank.empty() ? "" : " " + whiteRank + "";
			ret += " - ";
			ret += blackName.empty() ? "��: δ֪" : "��: " + blackName;
			ret += blackRank.empty() ? "" : " " + blackRank + "";
			ret += "  ";
		}
		

		if(winByResign)
		{
			if(winner == White)
			{
				ret+= "������ʤ";
			}
			else if(winner == Black)
			{
				ret+= "������ʤ";
			}
		}
		else
		{
			if(winner == White)
			{
				ret+= "��ʤ";
			}
			else if(winner == Black)
			{
				ret+= "��ʤ";
			}

			if(!result.empty())
			{
				ret+= result;
			}
		}

		//if(!komi.empty())
		//{
		//	ret+= " ��Ŀ: " + komi;
		//}
		return ret;
	}

	int GetRealMoveCount()
	{
		int n;
		for(n = 0; n < moveCount && moves[n].addition; n++)
		{}
		return moveCount - n;
	}
};

string RemoveExtension(const string & src)
{
	if(src.length() > 4 && src[src.length() - 4] == '.')
	{
		return src.substr(0, src.length() - 4);
	}
	else
	{
		return src;
	}
}

string ReadFileIntoString(const string & srcPath)
{
	string ret;
	FILE * file = fopen(srcPath.c_str(), "r");
	if(!file)
	{
		return ret;
	}
	while(!feof(file))
	{
		ret.push_back(fgetc(file));
	}
	fclose(file);
	return ret;
}

// ������������, SGF���õ�����ĸ
bool ReadMove(const string & src, int & i, Move & m)
{
	while(i < src.size() && src[i] <= 32)
	{
		i++;
	}

	if(i >= src.size() || src[i] != '[')
	{
		return false;
	}

	i++;
	if(i < src.size() && src[i] >= 'a' && src[i] <= 'z')
	{
		m.x = src[i] - 'a';
		i++;
		if(i < src.size() && src[i] >= 'a' && src[i] <= 'z')
		{
			m.y = src[i] - 'a';
			while(i < src.size() && src[i] != ']')
			{
				i++;
			}
			i++;
			return true;
		}
	}

	return false;
}

// ���ַ���, �μ�SGF�ٷ�����
// ����������������, ������û�д���UTF,BIG5�ȸ��ֱ���
string ReadStringValue(const string & src, int & i)
{
	string ret;
	while(i < src.size() && src[i] <= 32)
	{
		i++;
	}

	if(i >= src.size() || src[i] != '[')
	{
		return ret;
	}
	i++;

	while(i < src.size() && src[i] != ']')
	{
		if(src[i] == '\\')
		{
			i++;
			if(i < src.size())
			{
				ret.push_back(src[i]);
			}
		}
		else
		{
			ret.push_back(src[i]);
			if(src[i] < 0 && i + 1 < src.size())
			{
				i++;
				ret.push_back(src[i]);
			}
		}
		i++;
	}
	i++;
	return ret;
}

// ����SGF��ʽ. 
// (Ϊɶ���õ�������? ��Ϊ�Ҳ�����.)
bool LoadSimpleSGF(Game & game, const string & srcPath)
{
	bool bReadSize = false;
	// ��ʱ������Ҳ����Move��
	string src = ReadFileIntoString(srcPath);
	if(src.empty())
	{
		return false;
	}
	int i = 0;
	// ���� (;
	while(i < src.size() && ( src[i] <= 32 || src[i] == '(' || src[i] == ';'))
	{
		i++;
	}

	// ���ζ���־, ֱ������������
	while(i < src.size() && src[i] != ')')
	{
		while(i < src.size() && (src[i] <= 32 || src[i] == ';' || src[i] == '('))
		{
			i++;
		}
		
		string propName;

		while(i < src.size() && src[i] >= 'A' && src[i] <= 'Z')
		{
			propName.push_back(src[i++]);
		}

		if(propName == "GM")
		{
			// ����, ��������
			string s = ReadStringValue(src, i);
			int t = s.empty() ? 1 : atoi(s.c_str());
			if(t == 4)
			{
				game.gameType = Game::gameRenju;
				if(!bReadSize)
				{
					game.boardSize = 15;
				}
				printf("Game type is Renju.\n", t);
			}
			else
			{
#ifdef RENJU_ONLY

				printf("Unsupported game type : %d, treat as Renju.\n", t);
				game.gameType = Game::gameRenju;
				if(!bReadSize)
				{
					game.boardSize = 15;
				}

#else
				if(t != 1)
				{
					printf("Unsupported game type : %d, treat as Weiqi.\n", t);
				}
				else
				{
					printf("Game type is Weiqi.\n", t);
				}
				game.gameType = Game::gameGo;
				if(!bReadSize)
				{
					game.boardSize = 19;
				}
#endif
			}

		}
		else if(propName == "AB" || propName == "AW" || propName == "B" || propName == "W")
		{
			// ����/��������
			Move m;
			m.color = propName == "AB"  || propName == "B" ? Black : White;
			m.addition = propName == "AB" || propName == "AW" ? true : false;
			while(ReadMove(src, i, m))
			{
				game.moves[game.moveCount++] = m;
			}
		}
		else if(propName == "SZ")
		{
			// ���̴�С. Χ��һ����19/13/9, ������15. ��Ҳ������
			bReadSize = true;
			string s = ReadStringValue(src, i);
			if(!s.empty())
			{
				game.boardSize = atoi(s.c_str());
			}
			if(game.boardSize < 3 || game.boardSize > MAX_BOARD_SIZE)
			{
				game.boardSize = game.gameType == Game::gameRenju ? 15 : 19;
			}

		}
		else if(propName == "PB")
		{
			// �ڷ�����
			game.blackName = ReadStringValue(src, i);
		}
		else if(propName == "PW")
		{
			// �׷�����
			game.whiteName = ReadStringValue(src, i);
		}
		else if(propName == "BR")
		{
			// �ڷ���λ
			game.blackRank = ReadStringValue(src, i);
		}
		else if(propName == "WR")
		{
			// �׷���λ
			game.whiteRank = ReadStringValue(src, i);
		}
		else if(propName == "KM")
		{
			// ��Ŀ
			game.komi = ReadStringValue(src, i);
		}
		else if(propName == "RE")
		{
			// �Ծֽ��
			// [B+R]     ������ʤ
			// [W+R]     ������ʤ
			// [B+0.5]   ��ʤ0.5Ŀ
			// [W+100]   ��ʤ100Ŀ
			// [����]    �������, ����"���", "��ʤ��"��
			string re =  ReadStringValue(src, i);
			if(re.size() >= 2 && re[1] == '+')
			{
				if(re[0] == 'B' || re[0] == 'b')
				{
					game.winner = Black;
				}
				else if(re[0] == 'W' || re[0] == 'w')
				{
					game.winner = White;
				}
				
				if(re.size() >= 3 )
				{
					if(re[2] == 'R' || re[2] == 'r')
					{
						game.winByResign = true;
					}
					game.result = re.substr(2, -1);
				}
				else
				{
					game.result = re;
				}
			}
			else
			{
				game.result = re;
			}
		}
		else if(propName == "C")
		{
			// ע��, ע��, ��ע
			string comment = ReadStringValue(src, i);
			printf("comment at move %d: \n%s\n", game.moveCount, comment.c_str());
		}
		else
		{
			// ��������ֱ�Ӻ���, �������Ƿ���, A,B,C,D�ȷ�֧��ʶ��, ��Щ����̫������.
			printf("Ignore property : %s\n", propName.c_str());
			while(i < src.size() && src[i] != ']')
			{
				i++;
			}
			i++;
		}
	}

	return true;
}


// ��������㷨, ����, �˺��������ر���, û��Ҫ�Ż�
bool HasLiberty(int BS, StoneColor board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], bool flags[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int x, int y)
{
	flags[x][y] = true;
	if(x <(BS-1)) 
	{
		if( board[x][y] == board[x+1][y] && !flags[x+1][y])
		{
			if(HasLiberty(BS, board, flags, x + 1, y))
			{
				return true;
			}
		}
		else if(board[x+1][y] == None)
		{
			return true;
		}
	}
	if(x >0)
	{
		if( board[x][y] == board[x-1][y] && !flags[x-1][y])
		{
			if( HasLiberty(BS, board, flags, x - 1, y))
			{
				return true;
			}
		}
		else if(board[x-1][y] == None)
		{
			return true;
		}
	}

	if(y <(BS-1))
	{
		if( board[x][y] == board[x][y+1] && !flags[x][y+1])
		{
			if( HasLiberty(BS, board, flags, x, y+1))
			{
				return true;
			}
		}
		else if(board[x][y+1] == None)
		{
			return true;
		}
	}
	if(y > 0)
	{
		if( board[x][y] == board[x][y-1] && !flags[x][y-1])
		{
			if( HasLiberty(BS, board, flags, x, y-1))
			{
				return true;
			}
		}
		else if(board[x][y-1] == None)
		{
			return true;
		}
	}
	return false;
}


// ����Ƿ���Ҫ����
void CheckTakeStone(int BS, StoneColor board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int x, int y)
{
	bool flags[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
	memset(flags, 0, sizeof(flags));
	StoneColor c = board[x][y];
	if(!HasLiberty(BS, board, flags, x, y))
	{
		for(int i=0; i<BS; i++)
		{
			for(int j=0; j<BS; j++)
			{
				if(flags[i][j])
				{
					assert(board[i][j] == c); // ԭ������ɫһ���ͼ������ͬ, ��Ϊ��ͬһ����
					board[i][j] = None; // ����
				}
			}
		}
	}
}

// ���������Ӻ���, ����Ҫ����
bool PutStoneRenju(int BS, StoneColor board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], const Move & m)
{
	if(m.color == None)
	{
		return false;
	}

	if(m.x < 0 || m.y < 0|| m.x >= BS || m.y >= BS)
	{
		printf("Coord is out of board, treat as pass.\n");
		return false;
	}

	board[m.x][m.y] = m.color;
	return true;
}

// Χ�����Ӻ���, ��Ҫ�������
bool PutStoneWeiqi(int BS, StoneColor board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], const Move & m)
{
	if(m.color == None)
	{
		return false;
	}

	if(m.x < 0 || m.y < 0|| m.x >= BS || m.y >= BS)
	{
		printf("Coord is out of board, treat as pass.\n");
		return false;
	}

	board[m.x][m.y] = m.color;

	// �ĸ�������嶼��Ҫ����Ƿ�����
	if(m.x < (BS-1) && m.color != board[m.x+1][m.y])
	{
		CheckTakeStone(BS, board, m.x + 1, m.y);
	}

	if(m.x > 0 && m.color != board[m.x-1][m.y])
	{
		CheckTakeStone(BS, board, m.x - 1, m.y);
	}

	if(m.y < (BS-1) && m.color != board[m.x][m.y+1])
	{
		CheckTakeStone(BS,board, m.x, m.y+1);
	}

	if(m.y > 0 && m.color != board[m.x][m.y-1])
	{
		CheckTakeStone(BS,board, m.x, m.y-1);
	}

	// ��Ŀǰ�Ĺ����ƺ�����Ҫ�������

	return true;
}

bool PutStone(Game::GameType gt, int BS, StoneColor board[MAX_BOARD_SIZE][MAX_BOARD_SIZE], const Move & m)
{
	if(gt == Game::gameGo)
	{
		return PutStoneWeiqi(BS, board, m);
	}
	else
	{
		return PutStoneRenju(BS, board, m);
	}
}

static HFONT CreateNativeFont(const string & _fontName, int _fontSize, bool _bold)
{
	if(_fontSize < 1)
	{
		_fontSize = 1;
	}
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	lf.lfHeight = -_fontSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	if(_bold)
	{
		lf.lfWeight = FW_BOLD;
	}
	else
	{
		lf.lfWeight = FW_NORMAL;
	}

	//lf.lfItalic    = _font.italic ? TRUE : FALSE;
	//lf.lfUnderline = _font.underline ? TRUE : FALSE;
	//lf.lfStrikeOut = _font.strikethrough ? TRUE : FALSE;
	lf.lfCharSet   = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	if(_fontName.length() > 32)
	{
#	ifdef UNICODE
		wcscpy(lf.lfFaceName, L"SYSTEM");
#	else
		strcpy(lf.lfFaceName, "SYSTEM");
#	endif
	}
	else
	{
#	ifdef UNICODE
		wcscpy(lf.lfFaceName, _fontName.c_str());
#	else
		strcpy(lf.lfFaceName, _fontName.c_str());
#	endif
	}
	return ::CreateFontIndirect(&lf);
}

bool IsStar(int bs, int x, int y)
{
	switch(bs)
	{
	case 9:
		return (x == 2 || x == 6) && (y == 2 || y == 6);
	case 13:
		return (x == 3 || x == 9) && (y == 3 || y == 9) || x == 6 && y == 6;
	case 19:
		return (x == 3 || x == 9 || x == 15) && (y == 3 || y == 9 || y == 15);
	default:
		return false;
	}
}

// ������, �����ӻ�����, û�����ӻ��ս����.
// ע��ͼƬ�ǲ�͸����, ԭ�������ӵĵط����ս���㸲�Ǿ������ӵ�Ч��
void DrawCell(HDC hDC,  int BS, StoneColor color, int i, int j, int x0, int y0, int num = 0)
{
	Graphics * g = Graphics::FromHDC(hDC);
	Status status = g->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	g->SetPageUnit( Gdiplus::UnitPixel );
	if(color == White || color == Black)
	{
		// ������
		if(color == White)
		{
			::SelectObject(hDC, ::GetStockObject(WHITE_BRUSH));
			::SetTextColor(hDC, RGB(0, 0, 0));
			SolidBrush brush(Color::White);
			g->FillEllipse(&brush, x0+1 , y0+1  , CW()-2, CW()-2);
		}
		else
		{
			::SelectObject(hDC, ::GetStockObject(BLACK_BRUSH));
			::SetTextColor(hDC, RGB(255, 255, 255));
			SolidBrush brush(Color::Black);
			g->FillEllipse(&brush, x0+1 , y0+1 , CW()-2, CW()-2);
		}
		Pen pen(Color::Black);
		g->DrawEllipse(&pen, x0+1 , y0+1  , CW()-2, CW()-2);

		if(num > 0)
		{
			// ������
			char buf[100];
			sprintf(buf, "%d", num);
			RECT rect = {x0+1, y0, x0 + CW(), y0 + CW()};
			::DrawText(hDC, buf, strlen(buf), &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		}
	}
	else
	{
		// ���ս����
		int cx = x0 + CW() / 2;
		int cy = y0 + CW() / 2;
		int l, t, r, b;
		l = i > 0 ? x0 : cx;
		t = j > 0 ? y0 : cy;
		r = i < (BS-1) ?  x0 + CW() : cx;
		b = j < (BS-1) ?  y0 + CW() : cy;
		::MoveToEx(hDC, l, cy, NULL);
		::LineTo(hDC, r, cy);
		::MoveToEx(hDC, cx, t, NULL);
		::LineTo(hDC, cx, b);

		if(IsStar(BS, i, j))
		{
			// ����λ���
			::SelectObject(hDC, ::GetStockObject(BLACK_BRUSH));
			::Rectangle(hDC, cx-1, cy-1 , cx+2, cy+2);
		}
	}
	g->ReleaseHDC( hDC );
	delete g;

}

float frand()
{
	return float(rand()) / float(RAND_MAX);
}

// ���̵����ǹ�������, ��GIF���������ѹ���ʸ�
void DrawBoardBG(HDC hDC, RECT rect)
{
	float x0 = frand();
	float y0 = frand();
	float sx = frand() * 0.00001f + 0.00007f; 
	float sy = frand() * 0.1f + 0.07f;
	for(int i=rect.left; i<rect.right; i++)
	{
		for(int j=rect.top; j<rect.bottom; j++)
		{
			float x = (i) * sx + 1 + x0;
			float y = (j) * sy  + 1 + y0;
			float v = 0;
			for(int k=1; k<5; k++)
			{
				float m = float(1 << k);
				float v0 = SimplexNoise1234::noise(x*m, y*m) / m;
				//v0 = fabs(v0);
				v+= v0;
			}
			//float v = SimplexNoise1234::noise(x * 0.001f, y * 1.0f);
			//v/=1.5;
			v = (v+ 1.0f) * 0.5f;
					
			int r = v * 30 + 195; //210
			int g = v * 15 + 177; // 184
			int b = v * 8 + 134; // 140
			::SetPixel(hDC, i, j, RGB(r, g, b));
		}
	}
}


// �Ѵ�firstNum��lastNum���ŷ�ת����һ��GIF
bool Convert(Game & game, const string & dstPath, int firstNum, int lastNum)
{
	int BS = game.boardSize;
	StoneColor preBoard[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
	memset(preBoard, 0, sizeof(preBoard));
	StoneColor board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
	memset(board, 0, sizeof(board));
	HDC hDC = ::CreateCompatibleDC(NULL);
	//Graphics * g = Graphics::FromHDC(hDC);
	COLORREF bgColor = RGB(255, 255, 255);
	HBRUSH hBgBrush = ::CreateSolidBrush(bgColor);
	HBRUSH hOldBrush = (HBRUSH) ::SelectObject(hDC, hBgBrush);
	HFONT hMonoFont = CreateNativeFont("Lucida Console", 10, false);
	HFONT hNumFont = CreateNativeFont("Tahoma", 10, false);
	HFONT hTitleFont = CreateNativeFont("Tahoma", 14, false);
	HFONT hOldFont = (HFONT) ::SelectObject(hDC, hNumFont);
	::SetTextColor(hDC, RGB(0, 0, 0));
	::SetBkMode(hDC, TRANSPARENT);

	// �������Ӵ�С��λͼ, ������������滭����
	HBITMAP hBmpCell = ::CreateBitmap(CW(), CW(), 1, 32, NULL);

	// ����λͼ
	HBITMAP hBmpBoard = ::CreateBitmap(BW(BS), BH(BS), 1, 32, NULL);

	// �Ȱ�����λͼѡ��hDC, ��λͼ�ϻ�����
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hDC, hBmpBoard);

	// ������
	RECT rect= { 0, 0, BW(BS), BH(BS) };
	DrawBoardBG(hDC, rect);

	// ������, ����һֱ����, ���ڵ�ͼ�Ͼ���
	string title = game.GetTitle();
	::SelectObject(hDC, hTitleFont);
	rect.bottom = BOARD_TOP;
	::DrawText(hDC, title.c_str(), title.length(), &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	for(int i=0; i< BS; i++)
	{
		// ������
		char buf[32];

		rect.left = BOARD_LEFT + CW() * BS + 4;
		rect.right = BW(BS) - 2;
		rect.top = BOARD_TOP + CW() * i;
		rect.bottom = rect.top + CW();

		::SelectObject(hDC, hMonoFont);

		sprintf(buf, "%d", BS - i);
		::DrawText(hDC, buf, strlen(buf), &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		rect.left = BOARD_LEFT + CW() * i;
		rect.right = rect.left + CW();
		rect.top = BOARD_TOP + CW() * BS;
		rect.bottom = BH(BS) - 4;
		//::SelectObject(hDC, hNumFont);

		buf[0] = 'A' + i;
		buf[1] = 0;
		::DrawText(hDC, buf, 1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	}



	// ��������, �Ѿ��������̵���, ���������, �������û�������㼰������.
	// �ڻ�֮ǰ�Ȱ����Ӱں�.

	// ������
	int n;
	for(n = 0; n < game.moveCount && game.moves[n].addition; n++)
	{
		Move & m = game.moves[n];
		board[m.x][m.y] = m.color;
	}

	// ��һ����������ʵ�����׵����, �����ǲ�һ������㿪ʼ������, �������˵��
	int firstMoveIndex = n;

	int num = 0;
	vector<int> num_to_index;

	// ����ӳ��, ��Ϊ�м������pass�Ͷ�������, ���Կ��ܲ�������ӳ��.
	num_to_index.push_back(-1); 

	// ��firstNum֮ǰ���ŷ�����Ϊ����, ��������Ҫ����.
	// �����������, ���ǾͿ��Դ�����������ʼ������. �����ͷ��ʼ.
	for(n=firstMoveIndex; n<game.moveCount && num+1 < firstNum; n++)
	{
		Move & m = game.moves[n];
		m.addition = false; // �ѶԾֹ����еĶ������Ӷ�����ʵ�ʵ�����. ��ʱ���������ּ����Ӱڱ仯������, ������û��Ҫ֧��.
		bool put = PutStone(game.gameType, BS, board, m);
		if(put && !m.addition) // ֻ�����ӳɹ��ż�������, �䵽��������������Ϊ��PASS
		{
			num++;
			num_to_index.push_back(n);
		}
	}

	firstMoveIndex = n; // firstNum ��Ӧ��λ��, ��ֵ���ܺ�firstNum��ͬ, ��Ϊ�����Ӻ�PASS

	// �ں�������, �������ӻ�, �����ӻ�����, û���ӻ�������.
	for(int i=0; i< BS; i++)
	{
		for(int j=0; j< BS; j++)
		{
			DrawCell(hDC, BS, board[i][j], i, j, BOARD_LEFT + CW()*i, BOARD_TOP + CW() * j, 0);
		}
	}
	
	// ���ڳ�ʼ������, �������̵���, ������, ����, ����ͳ�ʼ����

	
	vector<CxImage *> images; // ��֡��ͼƬ

	CxImage * image = new CxImage(); // ��һ֡
	images.push_back(image);
	image->CreateFromHBITMAP(hBmpBoard); // ��һ֡�ǵ�ͼ
	hBmpBoard = NULL; // �Ժ�����������, ������Ҫ��ͼ. TODO: �����Ƿ���Ҫɾ��λͼ���? 
	image->SetFrameDelay(100); // ��һ֡�̶��ӳ�100. TODO: �Ƿ�Ӧ�ÿ�������?
	image = NULL;

	// ��ʼ����������֡��, ����lastNum��Ϊֹ.
	for(int i=firstMoveIndex; i<game.moveCount && num < lastNum; i++)
	{
		// �㷨�Ǻܼ򵥵�, �Ա���һ֡ǰ�������, �ѱ��˵ĸ��ӻ���ͼƬ��, ����ȥ
		memcpy(preBoard, board, sizeof(board)); // �ȼ�ס��һ֡ǰ������

		// ����
		Move & m = game.moves[i]; 
		m.addition = false;
		bool put = PutStone(game.gameType, BS, board, m);
		if(put && !m.addition)
		{
			num++;
			num_to_index.push_back(i);
		}
		
		// ������̱仯, ע�����������, ���Ա仯�ĵ�����Ƕ��.
		// ���ǰѸ�����ı仯�ֿ�, ÿ���仯��һ��С��ͼ.
		// �����ĺô������״���, ����ͼƬ�ߴ�Ҳ��С.
		// ȱ��Ҳ����һֱ��ڸ����, ����ӻ���ҽ���.
		// Ҳ���Ժ���Լ�һ��ѡ��, ֧�ֶ���仯��ϲ�...

		vector<Cross> changes;
		for(int i=0; i<BS; i++)
		{
			for(int j=0; j<BS; j++)
			{
				if(preBoard[i][j] != board[i][j])
				{
					Cross pt = {i, j};
					if(preBoard[i][j] == None)
					{
						// ���ӷ�����ǰ��, ��ʾ��Ч���������Ӻ�����
						changes.insert(changes.begin(), pt);
					}
					else
					{
						changes.push_back(pt);
					}
				}
			}
		}

		bool first = true; 
		for(int n=0; n<changes.size(); n++)
		{
			int i = changes[n].x;
			int j = changes[n].y;

			CxImage * prevImage = images.back();

			RECT rect= { 0, 0, CW(), CW() };

			if(first && g_options.numbers > 0 &&  num - g_options.numbers > 0)
			{
				// ��һ���仯������, ��Ҫ����������ʾ, �����Ȱѹ��ڵ�����ɾ��
				int removeNumber = num - g_options.numbers; 
				int removeIndex = num_to_index[removeNumber];
				Move & rm = game.moves[removeIndex];

				// ��hBmpCellѡΪ��ǰλͼ, �����滭���������ĸ���
				hOldBmp = (HBITMAP)::SelectObject(hDC, hBmpCell);	
				DrawBoardBG(hDC, rect);
				DrawCell(hDC, BS, board[rm.x][rm.y], rm.x, rm.y, 0, 0, 0);
				::SelectObject(hDC, hOldBmp);

				// ��Ϊ��������ȥ
				image = new CxImage();
				image->CreateFromHBITMAP(hBmpCell);
				image->SetOffset(BOARD_LEFT + rm.x * CW(), BOARD_TOP + rm.y * CW());
				image->SetFrameDelay(0); // �������ӳ���0, ��ʵ��������һ����С���ӳ�
				images.push_back(image);

			}
			
			// ��hBmpCellѡΪ��ǰλͼ, �����滭�仯�ĸ���
			hOldBmp = (HBITMAP)::SelectObject(hDC, hBmpCell);	
			DrawBoardBG(hDC, rect);
			::SelectObject(hDC, hNumFont);
			DrawCell(hDC, BS, board[i][j], i, j, 0, 0, m.addition || g_options.numbers <= 0 ? 0 : num);
			::SelectObject(hDC, hOldBmp);

			// ��Ϊ��������ȥ
			image = new CxImage();
			image->CreateFromHBITMAP(hBmpCell);
			image->SetOffset(BOARD_LEFT + i * CW(), BOARD_TOP + j * CW());
			image->SetFrameDelay(0); // �������ӳ���0, ��ʵ��������һ����С���ӳ�
			images.push_back(image);

			if(first)
			{
				if(num == firstNum)
				{
					prevImage->SetFrameDelay(150); // ��һ���ӳٹ̶�150(Ϊɶǰ����100?) TODO: �Ƿ�Ӧ�ÿ�������?
				}
				else
				{
					prevImage->SetFrameDelay(g_options.delay); // �������ӳٰ��û����õ���
				}

				first = false;
			}
		}
	}

	// ���һ֡�ӳٳ�һ��, 400, Ȼ���ٴӵ�һ֡��ʼ����
	images.back()->SetFrameDelay(400);

	// ��ɫ����, ������ò�����, �Ǿ���Ĭ�ϵ�ɫ��+����, Ч���ܲ�.
	// TODO: �����ǰ�����ͼƬ������, �����Ļ���ЩԪ�ػ��������, ��������0.
	//       ��֪����������û������, �Ƿ�Ӧ����һ�ŵ���ͼ��������? 
	CQuantizer q(256, 8); 
    for(int i=0; i<images.size(); i++)
	{
		q.ProcessImage(images[i]->GetDIB()); 
	}
     
	// �������Ľ�����ɵ�ɫ��
    RGBQUAD* ppal=(RGBQUAD*)calloc(256 *sizeof(RGBQUAD),1); 
    q.SetColorTable(ppal); 


	for(int i=0; i<images.size(); i++)
	{
		// �ڸ�֡��Ӧ�õ�ɫ��
		images[i]->DecreaseBpp(8, true, ppal, 256);
	}
	
	// �����GIF�ļ�
	CxIOFile cxIOFile;
	cxIOFile.Open(dstPath.c_str(), _T("wb"));
	CxImageGIF multiimage;

	multiimage.SetStdPalette();
	multiimage.SetLoops(0);
	multiimage.Encode(&cxIOFile, &images[0], images.size());

	for(int i=0; i<images.size(); i++)
	{
		delete images[i];
	}
	images.clear();

	if (ppal) free(ppal); 
	::SelectObject(hDC, hOldBmp);
	::SelectObject(hDC, hOldFont);
	::SelectObject(hDC, hOldBrush);
	::DeleteObject(hNumFont);
	::DeleteObject(hTitleFont);
	::DeleteObject(hMonoFont);
	::DeleteObject(hBmpCell);
	::DeleteObject(hBmpBoard); // TODO: ֮ǰ���ǰ�hBmpBoard��ΪNULL����?
	::DeleteObject(hBgBrush);
	::DeleteDC(hDC);
	return true;
}

// ��������, ������ת��һ������GIF
bool Convert(const string & srcPath)
{
	Game game;
	if(!LoadSimpleSGF(game, srcPath))
	{
		return false;
	}

	bool actualSplit = g_options.RealSplit();
	int finalNum = game.GetRealMoveCount();
	if(actualSplit)
	{
		bool ret = true;
		int firstNum = 1;
		if(g_options.splitPeriodically)
		{
			while(firstNum <= finalNum)
			{
				int lastNum = firstNum + g_options.splitPoints[0] - 1;
				char buf[32];
				sprintf(buf, "%03d",firstNum);
				string sFirst = buf;
				sprintf(buf, "%03d",lastNum+2 < finalNum ? lastNum+2 :finalNum);
				string sLast = buf;
				const string dstPath = RemoveExtension(srcPath) + "(" + sFirst + "-" + sLast + ").gif";
				if(!Convert(game, dstPath, firstNum, lastNum + 2))
				{
					ret = false;
				}
				firstNum = lastNum +1;
			}
		}
		else
		{
			for(int i=0; i<=g_options.splitCount && firstNum <= finalNum; i++)
			{
				int lastNum = i<g_options.splitCount? g_options.splitPoints[i] - 1 : 1999;
				char buf[32];
				sprintf(buf, "%03d",firstNum);
				string sFirst = buf;
				sprintf(buf, "%03d",lastNum+2 < finalNum ? lastNum+2 :finalNum);
				string sLast = buf;
				const string dstPath = RemoveExtension(srcPath) + "(" + sFirst + "-" + sLast + ").gif";
				if(!Convert(game, dstPath, firstNum, lastNum+2))
				{
					ret = false;
				}
				firstNum = lastNum +1;
			}
		}
		return ret;
	}
	else
	{
		const string dstPath = RemoveExtension(srcPath) + ".gif";
		return Convert(game, dstPath, 1, finalNum);
	}
}


// ���öԻ���, �õ����ϸ����͵Ĺ��ϼ���
HWND hWndEditDelay    = NULL;
HWND hWndEditNumbers  = NULL;
HWND hWndEditSplit    = NULL;
HWND hWndEditCW    = NULL;

string GetWindowString(HWND hWnd)
{
	char buf[2048];
	GetWindowText(hWnd, buf, 2048);
	return buf;
}

static BOOL CALLBACK DialogFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_INITDIALOG:
		{
			::SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)::LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(100)));
			::SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, 
				(LPARAM)(HICON)::LoadImage(GetModuleHandle(NULL),
				MAKEINTRESOURCE(100),
				IMAGE_ICON,
				16,
				16,
				LR_DEFAULTCOLOR));

			hWndEditDelay = ::GetDlgItem(hWnd, IDC_EDIT_DELAY);
			hWndEditNumbers = ::GetDlgItem(hWnd, IDC_EDIT_NUMBERS);
			hWndEditSplit = ::GetDlgItem(hWnd, IDC_EDIT_SPLIT);
			hWndEditCW = ::GetDlgItem(hWnd, IDC_EDIT_CW);

			::SetWindowText(hWndEditDelay, g_options.GetDelayString().c_str());
			::SetWindowText(hWndEditNumbers, g_options.GetNumbersString().c_str());
			::SetWindowText(hWndEditSplit, g_options.GetSplitString().c_str());
			::SetWindowText(hWndEditCW, g_options.GetCWString().c_str());

			return 1;
		}
		break;
	case WM_COMMAND:
		if(wParam == IDOK)
		{
			g_options.SetDelayString(GetWindowString(hWndEditDelay));
			g_options.SetNumbersString(GetWindowString(hWndEditNumbers));
			g_options.SetSplitString(GetWindowString(hWndEditSplit));
			g_options.SetCWString(GetWindowString(hWndEditCW));
			EndDialog(hWnd, IDOK);
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, IDCANCEL);
		return 1;
	}
	return 0;
}

void LoadSaveOptions(bool load)
{
	char buf[MAX_PATH];
	::GetModuleFileName(NULL, buf, MAX_PATH);
	string path = RemoveExtension(buf) + ".opt";
	if(load)
	{
		memset(&g_options, 0, sizeof(g_options));
		g_options.delay = 50;
		g_options.numbers = 1;
		g_options.splitCount = 0;
		g_options.cw = 23;
		FILE * file = fopen(path.c_str(), "rb");
		if(file)
		{
			fread(&g_options, sizeof(g_options), 1, file);
			fclose(file);
		}
	}
	else
	{
		FILE * file = fopen(path.c_str(), "wb+");
		if(file)
		{
			fwrite(&g_options, sizeof(g_options), 1, file);
			fclose(file);
		}
	}

}
bool ShowConfigDialog()
{
	InitCommonControls();
	LoadSaveOptions(true);
	INT_PTR ret = ::DialogBox(::GetModuleHandle(NULL), MAKEINTRESOURCE(200), NULL, (DLGPROC) DialogFunc);
	if(ret == IDOK)
	{
		LoadSaveOptions(false);
		return true;
	}
	else
	{
		return false;
	}
}


int _main(int argc, char * argv[])
{
	if(argc > 1)
	{
		if(!ShowConfigDialog())
		{
			printf("warning: failed to show options dialog.\n");
		}
		int ret = 0;
		for(int i=1; i<argc; i++)
		{
			if(!Convert(argv[i]))
			{
				ret++;
			}
		}
		return ret;
	}
	else
	{
		OPENFILENAME ofn; 
		TCHAR szFile[MAX_PATH];

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = "Smart Go Format\0*.sgf\0\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ;

		if(GetOpenFileName(&ofn)==TRUE)
		{
			if(!ShowConfigDialog())
			{
				return -5;
			}
			int ret = Convert(ofn.lpstrFile) ? 0 : -2;
			return ret;
		}
		else
		{
			return -1;
		}
	}
}

int main(int argc, char * argv[])
{
	srand(::GetTickCount());
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	int ret = _main(argc, argv);
	GdiplusShutdown(gdiplusToken);
	return ret;
}
