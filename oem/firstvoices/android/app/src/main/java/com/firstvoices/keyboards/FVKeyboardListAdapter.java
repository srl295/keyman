package com.firstvoices.keyboards;

import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.tavultesoft.kmea.KMManager;
import com.tavultesoft.kmea.data.KeyboardController;

class FVKeyboardListAdapter extends ArrayAdapter<FVShared.FVKeyboard> {

    Typeface listFont;

    private static class ViewHolder {
        ImageView check;
        TextView text1;
        ImageButton helpButton;
        ImageButton nextButton;
        String keyboardID;
    }

    FVKeyboardListAdapter(Context context, FVShared.FVRegion regionData) {
        super(context, 0, regionData.keyboards);
    }

    @NonNull
    @Override
    public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
        ViewHolder holder;

        FVShared.FVKeyboard keyboard = getItem(position);

        if(convertView == null) {
            convertView = LayoutInflater.from(getContext()).inflate(R.layout.keyboard_row_layout, parent, false);
            holder = new ViewHolder();
            holder.check = convertView.findViewById(R.id.image1);
            holder.text1 = convertView.findViewById(R.id.text1);
            holder.helpButton = convertView.findViewById(R.id.buttonHelp);
            holder.helpButton.setOnClickListener(new FVKeyboardListAdapter.FVOnClickHelpListener());
            holder.nextButton = convertView.findViewById(R.id.imageNext);
            holder.nextButton.setOnClickListener(new FVKeyboardListAdapter.FVOnClickNextListener());
            holder.keyboardID = keyboard.id;
            convertView.setTag(holder);

            if (listFont != null) {
                holder.text1.setTypeface(listFont, Typeface.BOLD);
            }
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        if(keyboard != null) {
            // Check if keyboard is installed
            if (KeyboardController.getInstance().keyboardExists(FVShared.FVDefault_PackageID, keyboard.id, null)) {
                holder.check.setVisibility(View.VISIBLE);
            }
            holder.text1.setText(keyboard.name);
            holder.helpButton.setTag(keyboard.id);
        }

        return convertView;
    }

    @Override
    public boolean areAllItemsEnabled() {
        return true;
    }

    @Override
    public boolean isEnabled(int position) {
        return true;
    }

    private class FVOnClickHelpListener implements ImageButton.OnClickListener {
        @Override
        public void onClick(View v) {
            FVShared.getInstance().helpAction(getContext(), (String)v.getTag());
        }
    }

    private class FVOnClickNextListener implements View.OnClickListener {
        @Override
        public void onClick(View v) {
            ViewHolder holder = (ViewHolder)v.getTag();
            Intent intent = new Intent(getContext(), FVKeyboardSettingsActivity.class);
            Bundle args = new Bundle();
            args.putString(KMManager.KMKey_KeyboardID, holder.keyboardID);
            intent.putExtras(args);
            getContext().startActivity(intent);
            //FVShared.getInstance().setCheckState(id, true);
        }
    }

}
