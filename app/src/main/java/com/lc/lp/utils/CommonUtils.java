package com.lc.lp.utils;

/**
 * 常用工具类
 * Created by lucas on 2021/11/18.
 */
public class CommonUtils {

    /**
     * 秒数转为时分秒
     * @param time
     * @return
     */
    public static String second2HM(long time) {

        long hour = time / 3600;
        long minute = (time % 3600) / 60;
        long second = (time % 3600) % 60;

        return (hour< 10 ? ("0" + hour) : hour) + ":" + (minute < 10 ? ("0" + minute) : minute) + ":" + (second < 10 ? ("0" + second) : second);
    }




}
