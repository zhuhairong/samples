package com.hilive.hiffmpeg;

import android.app.Activity;
import android.content.Context;
import android.net.Uri;
import android.view.WindowManager;

import androidx.annotation.NonNull;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Utils {
    public static String createTempPath(final Uri uri, @NonNull final Context context, @NonNull final String fileName, final boolean cover) {
        String filePath = null;
        InputStream inputStream = null;

        try {
            byte[] buffer = new byte[100 * 1024];

            inputStream = context.getContentResolver().openInputStream(uri);

            boolean newFile = true;
            File file = new File(context.getCacheDir(), fileName);
            if (file.exists()) {
                if (cover) {
                    file.delete();
                } else {
                    newFile = false;
                }
            }

            if (newFile) {
                OutputStream outputStream = new FileOutputStream(file);
                int ret = 0;
                while ((ret = inputStream.read(buffer)) != -1) {
                    outputStream.write(buffer, 0, ret);
                }
                outputStream.flush();

                try {
                    outputStream.close();
                } catch (IOException e) {

                }
            }
            filePath = file.getAbsolutePath();
        } catch (Exception e) {

        } finally {
            try {
                if (inputStream != null) {
                    inputStream.close();
                }
            } catch (Exception e) {
            }
        }

        return filePath;
    }

    /**
     * 根据手机的分辨率从 dp 的单位 转成为 px(像素)
     */
    public static int dip2px(Context context, float dpValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (dpValue * scale + 0.5f);
    }

    /**
     * 根据手机的分辨率从 px(像素) 的单位 转成为 dp
     */
    public static int px2dip(Context context, float pxValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (pxValue / scale + 0.5f);
    }

    public static int getDisplayWidth(Activity context){
        WindowManager wm = context.getWindowManager();

        int width = wm.getDefaultDisplay().getWidth();
        return width;
    }

    public static int getDisplayWHHeigth(Activity context){
        WindowManager wm = context.getWindowManager();

        int height = wm.getDefaultDisplay().getHeight();
        return height;
    }

    /**
     * 数字时间串
     * @return
     */
    public static String getDateNumber(){
        Date currentTime = new Date();
        SimpleDateFormat formatter = new SimpleDateFormat("yyyyMMddHHmmss");
        String dateString = formatter.format(currentTime);
        return dateString;
    }

    /**
     * 随机num个字母
     * @return
     */
    public static String randomCapital(int num){
        String chars = "abcdefghijklmnopqrstuvwxyz";
        StringBuffer buffer = new StringBuffer();
        for (int i =0; i< num; i++){
            buffer.append(chars.charAt((int)(Math.random() * 26)));
        }
        return  buffer.toString();
    }

    public static JSONObject parseJson(@NonNull String jsonStr) {
        try {
            return new JSONObject(jsonStr);
        } catch (JSONException e) {
            return new JSONObject();
        }
    }

    public static byte[] formatJson(JSONObject jsonObj) {
        return jsonObj.toString().getBytes();
    }
}
