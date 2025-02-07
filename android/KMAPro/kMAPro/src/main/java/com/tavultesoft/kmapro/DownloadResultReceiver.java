/**
 * Copyright (C) 2020 SIL International. All rights reserved.
 */
package com.tavultesoft.kmapro;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.ResultReceiver;
import android.widget.Toast;

import com.keyman.android.KmpInstallMode;
import com.tavultesoft.kmea.BaseActivity;
import com.tavultesoft.kmea.util.FileUtils;

import java.io.File;

public class DownloadResultReceiver extends ResultReceiver {
  private Context context;

  public DownloadResultReceiver(Handler handler, Context context) {
    super(handler);

    this.context = context;
  }

  @Override
  protected void onReceiveResult(int resultCode, Bundle resultData) {
    switch(resultCode) {
      case FileUtils.DOWNLOAD_ERROR :
        BaseActivity.makeToast(context, R.string.download_failed, Toast.LENGTH_SHORT);
        MainActivity.cleanupPackageInstall();
        break;
      case FileUtils.DOWNLOAD_SUCCESS :
        String downloadedFilename = resultData.getString("filename");
        String languageID = resultData.getString("language");
        String kmpFilename = resultData.getString("destination") + File.separator + downloadedFilename;
        KmpInstallMode installMode = (KmpInstallMode) resultData.getSerializable("installMode");
        if(installMode == null) installMode = KmpInstallMode.Full;

        Bundle bundle = new Bundle();
        bundle.putString("kmpFile", kmpFilename);
        bundle.putString("language", languageID);
        bundle.putSerializable("installMode", installMode);
        Intent packageIntent = new Intent(context, PackageActivity.class);
        packageIntent.putExtras(bundle);
        context.startActivity(packageIntent);
        break;
      case FileUtils.DOWNLOAD_CANCELLED :
        // NO-OP
        break;
    }
    super.onReceiveResult(resultCode, resultData);
  }
}