#define EXPTOSTRING1(x) #x
#define EXPTOSTRING(x) EXPTOSTRING1(x)
#ifdef DEFAULT_HASHSIZE
	std::cout << "\tDEFAULT_HASHSIZE " << EXPTOSTRING(DEFAULT_HASHSIZE) << "\n";
#endif
#ifdef DEFAULT_PAWNHASHSIZE
	std::cout << "\tDEFAULT_PAWNHASHSIZE " << EXPTOSTRING(DEFAULT_PAWNHASHSIZE) << "\n";
#endif
#ifdef DEFAULT_EVALCACHESIZE
	std::cout << "\tDEFAULT_EVALCACHESIZE " << EXPTOSTRING(DEFAULT_EVALCACHESIZE) << "\n";
#endif
#ifdef DEFAULT_BOOK
	std::cout << "\tDEFAULT_BOOK " << EXPTOSTRING(DEFAULT_BOOK) << "\n";
#endif
#ifdef USE_UNMAKE_MOVE
	std::cout << "\tUSE_UNMAKE_MOVE " << EXPTOSTRING(USE_UNMAKE_MOVE) << "\n";
#endif
#ifdef USE_IID
	std::cout << "\tUSE_IID " << EXPTOSTRING(USE_IID) << "\n";
#endif
#ifdef USE_PVS
	std::cout << "\tUSE_PVS " << EXPTOSTRING(USE_PVS) << "\n";
#endif
#ifdef USE_HISTORY
	std::cout << "\tUSE_HISTORY " << EXPTOSTRING(USE_HISTORY) << "\n";
#endif
#ifdef USE_KILLER
	std::cout << "\tUSE_KILLER " << EXPTOSTRING(USE_KILLER) << "\n";
#endif
#ifdef USE_NULLMOVE
	std::cout << "\tUSE_NULLMOVE " << EXPTOSTRING(USE_NULLMOVE) << "\n";
#endif
#ifdef USE_FUTILITYPRUNING
	std::cout << "\tUSE_FUTILITYPRUNING " << EXPTOSTRING(USE_FUTILITYPRUNING) << "\n";
#endif
#ifdef USE_EXTENDED_FUTILITYPRUNING
	std::cout << "\tUSE_EXTENDED_FUTILITYPRUNING " << EXPTOSTRING(USE_EXTENDED_FUTILITYPRUNING) << "\n";
#endif
#ifdef USE_RAZORING
	std::cout << "\tUSE_RAZORING " << EXPTOSTRING(USE_RAZORING) << "\n";
#endif
#ifdef USE_EVALCACHE
	std::cout << "\tUSE_EVALCACHE " << EXPTOSTRING(USE_EVALCACHE) << "\n";
#endif
#ifdef EXTEND_IN_CHECK
	std::cout << "\tEXTEND_IN_CHECK " << EXPTOSTRING(EXTEND_IN_CHECK) << "\n";
#endif
#ifdef EXTEND_RECAPTURE
	std::cout << "\tEXTEND_RECAPTURE " << EXPTOSTRING(EXTEND_RECAPTURE) << "\n";
#endif
#ifdef MAXDEPTH
	std::cout << "\tMAXDEPTH " << EXPTOSTRING(MAXDEPTH) << "\n";
#endif
#ifdef MAXPLY
	std::cout << "\tMAXPLY " << EXPTOSTRING(MAXPLY) << "\n";
#endif
#ifdef MOVELIST_MAXSIZE
	std::cout << "\tMOVELIST_MAXSIZE " << EXPTOSTRING(MOVELIST_MAXSIZE) << "\n";
#endif
#ifdef COLLECT_STATISTICS
	std::cout << "\tCOLLECT_STATISTICS " << EXPTOSTRING(COLLECT_STATISTICS) << "\n";
#endif
#ifdef STATS_MOVELIST
	std::cout << "\tSTATS_MOVELIST " << EXPTOSTRING(STATS_MOVELIST) << "\n";
#endif
#undef EXPTOSTRING
#undef EXPTOSTRING1
