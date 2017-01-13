
#ifndef MsgLoad
#define MsgLoad



class iTVPMessageResourceProvider {
public:
	virtual const tjs_char* GetMessage( tjs_int index, tjs_uint& length ) = 0;
	virtual void ReleaseMessage( const tjs_char* mes, tjs_int index ) = 0;
};

extern void TVPLoadMessage( iTVPMessageResourceProvider* p );

#endif // MsgLoad
