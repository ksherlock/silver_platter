#pragma noroot
#pragma optimize -1
#pragma lint -1

#include <timetool.h>
#include <misctool.h>
#include <intmath.h>

void tiTimeRec2GMTString(const TimeRecPtr t, char *str)
{
static const char weekday[] = "Sun,Mon,Tue,Wed,Thu,Fri,Sat,";
static const char month[] = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ";

int i;
LongWord secs;

tiPrefRec tiPrefs;
TimeRec tr;

  tiPrefs.pCount = 3;
  tiGetTimePrefs(&tiPrefs);

  secs = ConvSeconds(TimeRec2Secs, 0, (Pointer)t);

  secs += tiPrefs.secOffset;

#if 0
  //add daylight savings time...
  if (ReadBParam(0x5e) & 0x02 == 0) secs += 3600;
#endif

  ConvSeconds(secs2TimeRec, secs, (Pointer)&tr);

  str[0] = 29;

  i = (tr.weekDay - 1) << 2;

  // Day of week
  str[1] = weekday[i++];
  str[2] = weekday[i++];
  str[3] = weekday[i++];
  str[4] = weekday[i++];
  str[5] = ' ';

  // day
  Int2Dec(tr.day + 1, &str[6], 2, 0);
  if (str[6] == ' ') str[6] = '0';

  str[8] = ' ';

  i = tr.month << 2;
  str[9] = month[i++];
  str[10] = month[i++];
  str[11] = month[i++];
  str[12] = month[i++];

  // year
  Int2Dec(tr.year + 1900, &str[13], 4, 0);
  str[17] = ' ';

  Int2Dec(tr.hour, &str[18], 2, 0);
  if (str[18] == ' ') str[18] = '0';
  str[20] = ':';

  Int2Dec(tr.minute, &str[21], 2, 0);
  if (str[21] == ' ') str[21] = '0';
  str[23] = ':';

  Int2Dec(tr.hour, &str[24], 2, 0);
  if (str[24] == ' ') str[24] = '0';

  str[26] = ' ';
  str[27] = 'G';
  str[28] = 'M';
  str[29] = 'T';

}
