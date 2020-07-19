package com.hilive.hiffmpeg;

import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.nfc.Tag;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;

public class H264Decoder {
    private static final String TAG = "[hilive][h264decoder]";
    private static final int MEDIACODEC_INPUT_TIMEOUT_USEC = 2000000;
    private static final int MEDIACODEC_OUTPUT_TIMEOUT_USEC = 30000;
    private boolean inited = false;
    private MediaExtractor mediaExtractor = null;
    private MediaCodec mediaCodec = null;
    private long mDuration = 0;
    private long mCount = 0;
    private Integer mFps = 0;
    private long startMs = 0;
    private long lastMs = 0;
    private long mInputFrameCount = 0;
    private long mOutputFrameCount = 0;

    public boolean init(String filePath) {
        do {
            try {
                startMs = System.currentTimeMillis();
                mediaExtractor = new MediaExtractor();
                mediaExtractor.setDataSource(filePath);

                for (int i = 0; i < mediaExtractor.getTrackCount(); ++ i) {
                    MediaFormat format = mediaExtractor.getTrackFormat(i);
                    String mime = format.getString(MediaFormat.KEY_MIME);
                    if (mime.startsWith("video/")) {
                        mediaExtractor.selectTrack(i);
                        mediaCodec = MediaCodec.createDecoderByType(mime);
                        mediaCodec.configure(format, null, null, 0);
                        mFps = format.getInteger(MediaFormat.KEY_FRAME_RATE);
                        Log.i(TAG, "createDecoderByType " + mime + " format: " + format.toString());
                        break;
                    }
                }

                if (mediaCodec == null) {
                    break;
                }

                mediaCodec.start();
                mDuration = 1000 / mFps;
                inited = true;
            } catch (IOException e) {
                e.printStackTrace();
            }
        } while (false);

        return inited;
    }

    public void decodeLoop() {
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        boolean isEOS = false;

        while (true) {
            if (!isEOS) {
                int inIndex = mediaCodec.dequeueInputBuffer(MEDIACODEC_INPUT_TIMEOUT_USEC);
                if (inIndex >= 0) {
                    ByteBuffer buffer = mediaCodec.getInputBuffer(inIndex);
                    int sampleSize = mediaExtractor.readSampleData(buffer, 0);
                    if (sampleSize < 0) {
                        // We shouldn't stop the playback at this point, just pass the EOS
                        // flag to decoder, we will get it again from the
                        // dequeueOutputBuffer
                        Log.d(TAG, "InputBuffer BUFFER_FLAG_END_OF_STREAM");
                        mediaCodec.queueInputBuffer(inIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        isEOS = true;
                    } else {
                        mediaCodec.queueInputBuffer(inIndex, 0, sampleSize, mediaExtractor.getSampleTime(), 0);
                        mediaExtractor.advance();
                    }

                    ++ mInputFrameCount;
                } else {
                    Log.e(TAG, "dequeueInputBuffer fail " + inIndex);
                }
            }

            int outIndex = mediaCodec.dequeueOutputBuffer(info, MEDIACODEC_OUTPUT_TIMEOUT_USEC);
            switch (outIndex) {
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    Log.d(TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
                    break;
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    Log.d(TAG, "New format " + mediaCodec.getOutputFormat());
                    break;
                case MediaCodec.INFO_TRY_AGAIN_LATER:
                    Log.d(TAG, "dequeueOutputBuffer timed out!");
                    break;
                default:
                    ++ mOutputFrameCount;
                    ByteBuffer buffer = mediaCodec.getOutputBuffer(outIndex);
//                    Log.v(TAG, "We can't use this buffer but render it due to the API limit, " + buffer);
                    mediaCodec.releaseOutputBuffer(outIndex, true);
                    break;
            }

            // All decoded frames have been rendered, we can stop playing now
            if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                Log.d(TAG, "OutputBuffer BUFFER_FLAG_END_OF_STREAM");
                break;
            }
        }
    }

    public void uint() {
        try {
            lastMs = System.currentTimeMillis();
            long spent = lastMs - startMs;

            Log.i(TAG, "decode spent: " + spent + " input: " + mInputFrameCount + " output: " + mOutputFrameCount);

            inited = false;
            if (mediaCodec != null) {
                mediaCodec.stop();
                mediaCodec.release();
                mediaExtractor.release();
                mediaCodec = null;
                mediaExtractor = null;
            }
        } catch (Exception e) {
            e.printStackTrace();
            mediaCodec = null;
        }
    }
}
