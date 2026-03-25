#pragma once

// Define color codes
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define GRAY    "\033[90m"    
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

// Bold colors
#define BBLACK   "\033[1;30m"
#define BRED     "\033[1;31m"
#define BGREEN   "\033[1;32m"
#define BYELLOW  "\033[1;33m"
#define BBLUE    "\033[1;34m"
#define BMAGENTA "\033[1;35m"
#define BCYAN    "\033[1;36m"
#define BWHITE   "\033[1;37m"

// Background colors
#define BGBLACK   "\033[40m"
#define BGRED     "\033[41m"
#define BGGREEN   "\033[42m"
#define BGYELLOW  "\033[43m"
#define BGBLUE    "\033[44m"
#define BGMAGENTA "\033[45m"
#define BGCYAN    "\033[46m"
#define BGWHITE   "\033[47m"

struct GridElement
{
	GridElement() :
		GridElement(0, GRAY)
	{
	}

	GridElement(int inTypeID, const char* inColor) :
		m_TypeID{ inTypeID },
		m_Color{ inColor }
	{
	}

	int m_TypeID;
	const char* m_Color;
};