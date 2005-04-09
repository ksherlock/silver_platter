



void AppendHandle(Handle h1, Handle h2)
{
  LongWord size1, size2;

  if (!h1 || !h2) return;

  HUnlock(h1);

  size1 = GetHandleSize(h1);
  size2 = GetHandleSize(h2);

  if (size2)
  {
    SetHandleSize(h1, size1 + size2);
    HLock(h1);
    HandToPtr(h2, *h1 + size1, size2);
  }
}

{
Handle buffer;
word buffsize;

};


  if (sr.RcvQueued)
  {
    terr = TCPIPReadTCP(q->ipid, 2, (Ref)0, sr.RcvQueued, &rr);
    if (!q->buffer)
    {
      q->buffer = r.rrBuffHandle;
      q->buffSize = rr.rrBuffCount;
    }
    else
    {
      AppendHandle(q->buffer, rr.rrBuffHandle);
      q->buffSize += rr.rrBuffCount;
      DisposeHandle(rr.rrBuffHandle);
    }
  }
  if (q->buffSize)
  {

  }

}
