/*
   Copyright (C) 2003 Commonwealth Scientific and Industrial Research
   Organisation (CSIRO) Australia

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of CSIRO Australia nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE ORGANISATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "oggz/oggz.h"

#include "oggz_tests.h"

static long serialno1, serialno2;
static ogg_int64_t granulepos = 0;
static ogg_int64_t packetno = 0;

static int
hungry (OGGZ * oggz, int empty, void * user_data)
{
  unsigned char buf[1];
  ogg_packet op;
  long serialno = serialno1;
  long err;

  buf[0] = 'x';

  op.packet = buf;
  op.bytes = 1;
  op.granulepos = granulepos;
  op.packetno = packetno;

  switch (packetno) {
  case 0:
    INFO ("Feeding stream suffix");
    op.b_o_s = 0;
    op.e_o_s = 0;
    serialno = serialno1;
    break;
  case 1:
    op.b_o_s = 0;
    op.e_o_s = 1;
    serialno = serialno1;
    break;
  case 2:
    op.b_o_s = 0;
    op.e_o_s = 1;
    serialno = serialno2;
    break;
  }

#ifdef DEBUG
  fprintf (stderr, "Writing packet %d, serialno %d, bos=%d, eos=%d1\n",
	   packetno, (serialno==serialno1)?1:2, op.b_o_s, op.e_o_s);
#endif
  
  err = oggz_write_feed (oggz, &op, serialno, 0, NULL);
  if (err != 0) {
    switch (err) {
    case OGGZ_ERR_BAD_SERIALNO:
      FAIL ("Incorrect failure writing non-bos packet of new serialno");
      break;
    case OGGZ_ERR_EOS:
      FAIL ("Incorrect failure writing eos packet of stream suffix");
      break;
    default:
      FAIL ("Could not feed OGGZ");
      break;
    }
  }

  granulepos++;
  packetno++;

  if (packetno > 2) {
    return OGGZ_STOP_OK; /* Cancel write */
  } else {
    return OGGZ_CONTINUE;
  }
}

int
main (int argc, char * argv[])
{
  OGGZ * oggz;
  unsigned char buf[1];
  long n;

  oggz = oggz_new (OGGZ_WRITE|OGGZ_SUFFIX);
  if (oggz == NULL)
    FAIL("newly created OGGZ == NULL");

  serialno1 = oggz_serialno_new (oggz);
  serialno2 = oggz_serialno_new (oggz);

  if (oggz_write_set_hungry_callback (oggz, hungry, 1, NULL) != 0)
    FAIL("Could not set hungry callback");

  while ((n = oggz_write_output (oggz, buf, 1)) > 0);

  if (oggz_close (oggz) != 0)
    FAIL("Could not close OGGZ");

  exit (0);
}
