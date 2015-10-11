package org.toxiclab.droidg2;

import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class AdvanceSearch extends Activity {
	
	private EditText txtName;
	private EditText txtMin;
	private EditText txtMax;
	private EditText txtExt;
	private Button btnType;
	private Button btnSearch;
	
	private String fileType = "any";
	private String[] item;
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.advsearch);
        
        Resources res = getResources();
		item = res.getStringArray(R.array.asmedia);
        
        
        txtName = (EditText)findViewById(R.id.editText1);
        txtMin = (EditText)findViewById(R.id.editText2);
        txtMax = (EditText)findViewById(R.id.editText3);
        txtExt = (EditText)findViewById(R.id.editText5);
        btnType = (Button)findViewById(R.id.button1);
        btnSearch = (Button)findViewById(R.id.button2);
        
        btnType.setText(item[0]);
        btnType.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				
		    	AlertDialog.Builder builder = new AlertDialog.Builder(AdvanceSearch.this);
		    	builder.setTitle(getString(R.string.asmediatitle));
		    	builder.setSingleChoiceItems(item, -1, new DialogInterface.OnClickListener() {
					
					@Override
					public void onClick(DialogInterface dialog, int which) {
						// TODO Auto-generated method stub
						switch(which){
						case 0:
							fileType = "any";
							btnType.setText(item[0]);
							break;
						case 1:
							fileType = "video";
							btnType.setText(item[1]);
							break;
						case 2:
							fileType = "audio";
							btnType.setText(item[2]);
							break;
						case 3:
							fileType = "pic";
							btnType.setText(item[3]);
							break;
						case 4:
							fileType = "doc";
							btnType.setText(item[4]);
							break;
						case 5:
							fileType = "store";
							btnType.setText(item[5]);
							break;
						case 6:
							fileType = "virus";	
							btnType.setText(item[6]);
							break;
						}
					}
				});
		    	
		    	AlertDialog alert = builder.create();
		    	alert.show();
			}
        	
        });
        
        btnSearch.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				String name_ = txtName.getText().toString();
				String min_ = txtMin.getText().toString();
				String max_ = txtMax.getText().toString();
				String ext_ = txtExt.getText().toString();
				
				main._self.createSearch(name_, min_, max_, fileType, ext_);
				
				Toast toast = Toast.makeText(AdvanceSearch.this, String.format("Search \"%s\" has been added.", name_), Toast.LENGTH_LONG);
				toast.show();

				finish();
			}
       	
        });
	}
	
    public void onPause(){
    	super.onPause();
    }
    
    public void onResume(){
    	super.onResume();
    }
    
    public void onStop(){
    	super.onStop();
    }
    
}
