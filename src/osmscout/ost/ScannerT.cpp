/*
  This source is part of the libosmscout library
  Copyright (C) 2011  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

/*
  This source is part of the libosmscout library
  Copyright (C) 2011  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

#include <cstring>

#include <osmscout/ost/ScannerT.h>

namespace osmscout {
namespace ost {


// string handling, wide character

char* coco_string_create(const char* value) {
  char* data;
  int len = 0;
  if (value) { len = strlen(value); }
  data = new char[len + 1];
  strncpy(data, value, len);
  data[len] = 0;
  return data;
}

char* coco_string_create(const char *value , int startIndex, int length) {
  int len = 0;
  char* data;

  if (value) { len = length; }
  data = new char[len + 1];
  strncpy(data, &(value[startIndex]), len);
  data[len] = 0;

  return data;
}

void coco_string_delete(char* &data) {
  delete [] data;
  data = NULL;
}

Token::Token()
: kind(0),
  pos(0),
  col(0),
  line(0),
  val(NULL),
  next(NULL)
{
 // no code
}

Token::~Token()
{
  delete [] val;
}

Buffer::Buffer(const unsigned char* buf, int len)
{
  this->buf = new unsigned char[len];
  memcpy(this->buf, buf, len*sizeof(unsigned char));
  bufLen = len;
  bufPos = 0;
}

Buffer::~Buffer()
{
  delete [] buf;
}

int Buffer::Peek()
{
  int curPos = GetPos();
  int ch = Read();
  SetPos(curPos);
  return ch;
}

int Buffer::GetPos()
{
  return bufPos;
}

void Buffer::SetPos(int value) {
  if ((value < 0) || (value > bufLen)) {
    printf("--- buffer out of bounds access, position: %d\n", value);
    exit(1);
  }

  bufPos = value;
}

int Buffer::Read()
{
  if (bufPos < bufLen) {
    return buf[bufPos++];
  } else {
    return EoF;
  }
}

ScannerT::ScannerT(const unsigned char* buf, int len) {
  buffer = new Buffer(buf, len);
  Init();
}

ScannerT::~ScannerT() {
  delete [] tval;
  delete buffer;
}

void ScannerT::Init() {
  EOL    = '\n';
  eofSym = 0;
	maxT = 32;
	noSym = 32;
	int i;
	for (i = 65; i <= 90; ++i) start.set(i, 1);
	for (i = 97; i <= 122; ++i) start.set(i, 1);
	for (i = 48; i <= 57; ++i) start.set(i, 2);
	start.set(34, 3);
	start.set(61, 14);
	start.set(40, 7);
	start.set(41, 8);
	start.set(33, 15);
	start.set(91, 11);
	start.set(44, 12);
	start.set(93, 13);
		start.set(Buffer::EoF, -1);
	keywords.set("OST", 4);
	keywords.set("END", 5);
	keywords.set("TYPES", 6);
	keywords.set("TYPE", 7);
	keywords.set("OR", 11);
	keywords.set("OPTIONS", 12);
	keywords.set("AND", 13);
	keywords.set("IN", 17);
	keywords.set("EXISTS", 21);
	keywords.set("NODE", 22);
	keywords.set("WAY", 23);
	keywords.set("AREA", 24);
	keywords.set("RELATION", 25);
	keywords.set("ROUTE", 26);
	keywords.set("INDEX", 27);
	keywords.set("CONSUME_CHILDREN", 28);
	keywords.set("OPTIMIZE_LOW_ZOOM", 29);
	keywords.set("IGNORE", 30);
	keywords.set("MULTIPOLYGON", 31);


  tvalLength = 128;
  tval = new char[tvalLength]; // text of current token

  pos = -1; line = 1; col = 0;
  oldEols = 0;
  NextCh();
  if (ch == 0xEF) { // check optional byte order mark for UTF-8
    NextCh(); int ch1 = ch;
    NextCh(); int ch2 = ch;
    if (ch1 != 0xBB || ch2 != 0xBF) {
      printf("Illegal byte order mark at start of file");
      exit(1);
    }
    NextCh();
  }


  pt = tokens = CreateToken(); // first token is a dummy
}

void ScannerT::NextCh() {
  if (oldEols > 0) {
    ch = EOL;
    oldEols--;
  }
  else {
    pos = buffer->GetPos();
    ch = buffer->Read(); col++;
    // replace isolated '\r' by '\n' in order to make
    // eol handling uniform across Windows, Unix and Mac
    if (ch == '\r' && buffer->Peek() != '\n') {
      ch = EOL;
    }
    if (ch == EOL) {
      line++;
      col = 0;
    }
  }

}

void ScannerT::AddCh() {
  if (tlen >= tvalLength) {
    tvalLength *= 2;
    char *newBuf = new char[tvalLength];
    memcpy(newBuf, tval, tlen*sizeof(char));
    delete [] tval;
    tval = newBuf;
  }
  if (ch != Buffer::EoF) {
		tval[tlen++] = ch;
    NextCh();
  }
}


bool ScannerT::Comment0() {
	int level = 1, pos0 = pos, line0 = line, col0 = col, charPos0 = charPos;
	NextCh();
	if (ch == '/') {
		NextCh();
		for(;;) {
			if (ch == 10) {
				level--;
				if (level == 0) { oldEols = line - line0; NextCh(); return true; }
				NextCh();
			} else if (ch == buffer->EoF) return false;
			else NextCh();
		}
	} else {
		buffer->SetPos(pos0); NextCh(); line = line0; col = col0; charPos = charPos0;
	}
	return false;
}

bool ScannerT::Comment1() {
	int level = 1, pos0 = pos, line0 = line, col0 = col, charPos0 = charPos;
	NextCh();
	if (ch == '*') {
		NextCh();
		for(;;) {
			if (ch == '*') {
				NextCh();
				if (ch == '/') {
					level--;
					if (level == 0) { oldEols = line - line0; NextCh(); return true; }
					NextCh();
				}
			} else if (ch == '/') {
				NextCh();
				if (ch == '*') {
					level++; NextCh();
				}
			} else if (ch == buffer->EoF) return false;
			else NextCh();
		}
	} else {
		buffer->SetPos(pos0); NextCh(); line = line0; col = col0; charPos = charPos0;
	}
	return false;
}


Token* ScannerT::CreateToken() {
  Token *t;

  t = new Token();

  return t;
}

void ScannerT::AppendVal(Token *t) {
  delete [] t->val;
  t->val = new char[tlen+1];

  strncpy(t->val, tval, tlen);
  t->val[tlen] = '\0';
}

Token* ScannerT::NextToken() {
  while (ch == ' ' ||
			(ch >= 9 && ch <= 10) || ch == 13
  ) NextCh();
	if ((ch == '/' && Comment0()) || (ch == '/' && Comment1())) return NextToken();
  int recKind = noSym;
  int recEnd = pos;

  t = CreateToken();
  t->pos = pos; t->col = col; t->line = line; t->charPos = charPos;
  int state = start.state(ch);
  tlen = 0; AddCh();

  switch (state) {
  case -1: { t->kind = eofSym; break; } // NextCh already done
  case 0: {
    case_0:
    if (recKind != noSym) {
      tlen = recEnd - t->pos;
      SetScannerBehindT();
    }

    t->kind = recKind; break;
  } // NextCh already done
		case 1:
			case_1:
			recEnd = pos; recKind = 1;
			if ((ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= 'a' && ch <= 'z')) {AddCh(); goto case_1;}
			else {t->kind = 1; char *literal = coco_string_create(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}
		case 2:
			case_2:
			recEnd = pos; recKind = 2;
			if ((ch >= '0' && ch <= '9')) {AddCh(); goto case_2;}
			else {t->kind = 2; break;}
		case 3:
			case_3:
			if (ch <= '!' || (ch >= '#' && ch <= '[') || (ch >= ']' && ch <= 65535)) {AddCh(); goto case_3;}
			else if (ch == '"') {AddCh(); goto case_4;}
			else if (ch == 92) {AddCh(); goto case_5;}
			else {goto case_0;}
		case 4:
			case_4:
			{t->kind = 3; break;}
		case 5:
			case_5:
			if (ch <= '!' || (ch >= '#' && ch <= '[') || (ch >= ']' && ch <= 65535)) {AddCh(); goto case_3;}
			else if (ch == 92) {AddCh(); goto case_5;}
			else if (ch == '"') {AddCh(); goto case_6;}
			else {goto case_0;}
		case 6:
			case_6:
			recEnd = pos; recKind = 3;
			if (ch <= '!' || (ch >= '#' && ch <= '[') || (ch >= ']' && ch <= 65535)) {AddCh(); goto case_3;}
			else if (ch == '"') {AddCh(); goto case_4;}
			else if (ch == 92) {AddCh(); goto case_5;}
			else {t->kind = 3; break;}
		case 7:
			{t->kind = 9; break;}
		case 8:
			{t->kind = 10; break;}
		case 9:
			case_9:
			{t->kind = 15; break;}
		case 10:
			case_10:
			{t->kind = 16; break;}
		case 11:
			{t->kind = 18; break;}
		case 12:
			{t->kind = 19; break;}
		case 13:
			{t->kind = 20; break;}
		case 14:
			recEnd = pos; recKind = 8;
			if (ch == '=') {AddCh(); goto case_9;}
			else {t->kind = 8; break;}
		case 15:
			recEnd = pos; recKind = 14;
			if (ch == '=') {AddCh(); goto case_10;}
			else {t->kind = 14; break;}

  }
  AppendVal(t);

  return t;
}

// get the next token (possibly a token already seen during peeking)
Token* ScannerT::Scan() {
  if (tokens->next == NULL) {
    return pt = tokens = NextToken();
  } else {
    pt = tokens = tokens->next;
    return tokens;
  }
}

void ScannerT::SetScannerBehindT()
{
  buffer->SetPos(t->pos);
  NextCh();
  line = t->line; col = t->col; charPos = t->charPos;
  for (int i = 0; i < tlen; i++) NextCh();
}


// peek for the next token, ignore pragmas
Token* ScannerT::Peek() {
  do {
    if (pt->next == NULL) {
      pt->next = NextToken();
    }
    pt = pt->next;
  } while (pt->kind > maxT); // skip pragmas

  return pt;
}

// make sure that peeking starts at the current scan position
void ScannerT::ResetPeek() {
  pt = tokens;
}

} // namespace
} // namespace


