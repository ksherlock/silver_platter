extern Handle HandleFM;
extern Handle HandleTE;
extern Handle HandleSF;

extern Boolean FlagQDAux;
extern Boolean FlagFM;
extern Boolean FlagTE;
extern Boolean FlagSF;
extern Boolean FlagTT;
extern Boolean FlagTCP;

extern Word MyID;
extern Word Ipid;

extern Word FlagHTTP;

// functions
extern Word LoadNDATools(Word);
extern void UnloadNDATools(void);

extern Word StartServer(void);
extern Word StopServer(void);
extern void Server(void);
extern void ResetServer(void);

extern void InsertString(Word, const char *);
