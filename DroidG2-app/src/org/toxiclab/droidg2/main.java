package org.toxiclab.droidg2;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteOrder;
import java.util.Timer;

import org.toxiclab.droidg2.jni.toxiclab_download;
import org.toxiclab.droidg2.jni.toxiclab_downloadResult;
import org.toxiclab.droidg2.jni.toxiclab_hub;
import org.toxiclab.droidg2.jni.toxiclab_incoming;
import org.toxiclab.droidg2.jni.toxiclab_search;
import org.toxiclab.droidg2.jni.toxiclab_searchResult;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;


public class main extends Activity {
    /** Called when the activity is first created. */
    
    private static Thread server_thread;
    private File root;
    private static AssetManager am = null;
    private boolean first_time = false;
    
    private String _conn;
    private String _dconn;
    
    private Button status;
    private Button search;
    private Button download;
    private Button web; 
    private Button incoming;
    private Button connect;
   
    public static Handler m_handler = new Handler();
    public static Timer m_timer = new Timer();
    
    public static main _self;
    
    private NotificationManager mNotificationManager;
	private int SIMPLE_NOTFICATION_ID;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        if(!checkEndian()){
        	Toast.makeText(getApplicationContext(), "The application requires little endian to work. Exiting.", Toast.LENGTH_LONG).show();
        	return;
        }
        String state = Environment.getExternalStorageState();
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
        	Toast.makeText(getApplicationContext(), "Please insert SD card before running this application.",
			          Toast.LENGTH_LONG).show();
        	return;
        }
        
        setContentView(R.layout.main);
        
        _conn = getString(R.string.connect);
        _dconn = getString(R.string.Disconnect);
        
        _self = this;
        
        root = Environment.getExternalStorageDirectory();
        root = new File(root, "Android/data/org.toxiclab.droidg2");
        if(!root.exists()){
        	root.mkdir();
        	first_time = true;
        }
        
//      AdManager.setTestDevices( new String[] {
//		AdManager.TEST_EMULATOR,
//		// Android emulator
//		"84953AE3C4A0B2D18E775377ED1CCA40", // My T-Mobile G1 Test Phone
//		} );
        
        
        am = getAssets();
        
        copy_webui();
        copy_config();
        
        init_server();
        
        //create a button
        status = (Button)findViewById(R.id.button1);
        search = (Button)findViewById(R.id.button2);
        download = (Button)findViewById(R.id.button3);
        web = (Button)findViewById(R.id.button4);
        incoming = (Button)findViewById(R.id.button5);
        connect = (Button)findViewById(R.id.button6);
        
        connect.setOnClickListener(new OnClickListener(){
        	public void onClick(View v){
        		stop_server();
        		finish();
        	}
        });
        
        status.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				Intent intent = new Intent();
				intent.setClass(main.this, ConnectionActivity.class);
				startActivity(intent);
			}
        	
        });
        
        search.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				Intent intent = new Intent();
				intent.setClass(main.this, SearchActivity.class);
				startActivity(intent);
			}
        	
        });
        
        
        download.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				Intent intent = new Intent();
				intent.setClass(main.this, DownloadActivity.class);
				startActivity(intent);
			}
        	
        });
        
        web.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				String url = "http://127.0.0.1:4080/";
				Intent i = new Intent(Intent.ACTION_VIEW);
				i.setData(Uri.parse(url));
				startActivity(i);
			}
        	
        });
        
        incoming.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				Intent i = new Intent();
				i.setClass(main.this, IncomingActivity.class);
				startActivity(i);
			}
        	
        });
    }
    
    public boolean checkEndian(){
    	if(ByteOrder.LITTLE_ENDIAN.equals(ByteOrder.nativeOrder()))
    		return true;
    	return false;
    }
    
    public void onPause(){
    	super.onPause(); 
    }
    
    public void onStop(){
    	super.onStop();
    }
 
    public native void start_sharelin(String root);
    public native void createSearch(String dn, String min, String max, String media, String ext);
    public native toxiclab_search[] showSearch();
    public native toxiclab_searchResult[] showSearchResult(int id, String sortBy, boolean forward);
    public native toxiclab_hub[] showHubs();
    public native toxiclab_hub[] showHandshake();
    public native toxiclab_downloadResult showDownload(int id);
    public native toxiclab_download[] showAllDownload(String sortBy, boolean forward);
    public native boolean createDownload(int searchID, int resultID);
    public native toxiclab_incoming[] showIncoming();
    public native void pauseDownload(int id);
    public native void resumeDownload(int id);
    public native void deleteDownload(int id);
    public native void pauseSearch(int id);
    public native void deleteSearch(int id);
    public native void resumeSearch(int id);
    public native void killCore();
    
    public boolean init_server(){
    	if(server_thread!= null){
    		if(server_thread.getState() == Thread.State.TERMINATED)
    			server_thread = null;
    	}
    	
    	if(server_thread == null){
	    	server_thread = new Thread(new Runnable(){
	    		public void run(){
	    			String path = root.getAbsolutePath();
	    			start_sharelin(path);
	    		}
	    	});
	    	server_thread.start();
	    	initNotification();
	    	return true;
    	}
    	return false;
    }
    
    public void copy_webui(){
    	if(!first_time) return;
    	try{
	    	File temp = new File(root, "webui");
	    	if(temp.exists())
	    		return;
	    	temp.mkdir();
	    	copyFileFromAsset(root, "webui/readme.txt");
	    	copyFileFromAsset(root, "webui/status.html");
	    	
	    	temp = new File(root, "webui/default");
	    	temp.mkdir();
	    	//copy to webui/default
	    	copyFileFromAsset(root, "webui/default/blank.tpl");
	    	copyFileFromAsset(root, "webui/default/chunk.tpl");
	    	copyFileFromAsset(root, "webui/default/dlinfo.tpl");
	    	copyFileFromAsset(root, "webui/default/downloaders.tpl");
	    	copyFileFromAsset(root, "webui/default/downloads.tpl");
	    	copyFileFromAsset(root, "webui/default/fancydark.tpl");
	    	copyFileFromAsset(root, "webui/default/fancylight.tpl");
	    	copyFileFromAsset(root, "webui/default/home.tpl");
	    	copyFileFromAsset(root, "webui/default/incoming.tpl");
	    	copyFileFromAsset(root, "webui/default/linkform.tpl");
	    	copyFileFromAsset(root, "webui/default/log.tpl");
	    	copyFileFromAsset(root, "webui/default/menu.tpl");
	    	copyFileFromAsset(root, "webui/default/page.tpl");
	    	copyFileFromAsset(root, "webui/default/pageprefs.tpl");
	    	copyFileFromAsset(root, "webui/default/results.tpl");
	    	copyFileFromAsset(root, "webui/default/resultsgroup.tpl");
	    	copyFileFromAsset(root, "webui/default/searches.tpl");
	    	copyFileFromAsset(root, "webui/default/searchform.tpl");
	    	copyFileFromAsset(root, "webui/default/selectpage.tpl");
	    	copyFileFromAsset(root, "webui/default/settings.tpl");
	    	copyFileFromAsset(root, "webui/default/share.tpl");
	    	copyFileFromAsset(root, "webui/default/status.tpl");
	    	copyFileFromAsset(root, "webui/default/template.conf");
	    	copyFileFromAsset(root, "webui/default/template.css");
	    	copyFileFromAsset(root, "webui/default/text.tpl");
	    	copyFileFromAsset(root, "webui/default/uploads.tpl");
	    	
	    	temp = new File(root, "webui/dos");
	    	temp.mkdir();
	    	
	    	copyFileFromAsset(root, "webui/dos/blank.tpl");
	    	copyFileFromAsset(root, "webui/dos/chunk.tpl");
	    	copyFileFromAsset(root, "webui/dos/dlinfo.tpl");
	    	copyFileFromAsset(root, "webui/dos/downloaders.tpl");
	    	copyFileFromAsset(root, "webui/dos/downloads.tpl");
	    	copyFileFromAsset(root, "webui/dos/fancydark.tpl");
	    	copyFileFromAsset(root, "webui/dos/fancylight.tpl");
	    	copyFileFromAsset(root, "webui/dos/home.tpl");
	    	copyFileFromAsset(root, "webui/dos/incoming.tpl");
	    	copyFileFromAsset(root, "webui/dos/linkform.tpl");
	    	copyFileFromAsset(root, "webui/dos/log.tpl");
	    	copyFileFromAsset(root, "webui/dos/menu.tpl");
	    	copyFileFromAsset(root, "webui/dos/page.tpl");
	    	copyFileFromAsset(root, "webui/dos/pageprefs.tpl");
	    	copyFileFromAsset(root, "webui/dos/results.tpl");
	    	copyFileFromAsset(root, "webui/dos/resultsgroup.tpl");
	    	copyFileFromAsset(root, "webui/dos/searches.tpl");
	    	copyFileFromAsset(root, "webui/dos/searchform.tpl");
	    	copyFileFromAsset(root, "webui/dos/selectpage.tpl");
	    	copyFileFromAsset(root, "webui/dos/settings.tpl");
	    	copyFileFromAsset(root, "webui/dos/share.tpl");
	    	copyFileFromAsset(root, "webui/dos/status.tpl");
	    	copyFileFromAsset(root, "webui/dos/uploads.tpl");
	    	
	    	temp = new File(root, "webui/default/img");
	    	temp.mkdir();
	    	copyFileFromAsset(root, "webui/default/img/delete16.png");
	    	copyFileFromAsset(root, "webui/default/img/down16.png");
	    	copyFileFromAsset(root, "webui/default/img/download.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-archive.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-audio.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-document.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-iso.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-msapp.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-picture.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-unknown.png");
	    	copyFileFromAsset(root, "webui/default/img/ext-video.png");
	    	copyFileFromAsset(root, "webui/default/img/fav.png");
	    	copyFileFromAsset(root, "webui/default/img/folder.png");
	    	copyFileFromAsset(root, "webui/default/img/hint.png");
	    	copyFileFromAsset(root, "webui/default/img/kill.png");
	    	copyFileFromAsset(root, "webui/default/img/log.png");
	    	copyFileFromAsset(root, "webui/default/img/overview.png");
	    	copyFileFromAsset(root, "webui/default/img/readme");
	    	copyFileFromAsset(root, "webui/default/img/remove-file.png");
	    	copyFileFromAsset(root, "webui/default/img/resume16.png");
	    	copyFileFromAsset(root, "webui/default/img/search.png");
	    	copyFileFromAsset(root, "webui/default/img/settings.png");
	    	copyFileFromAsset(root, "webui/default/img/share.png");
	    	copyFileFromAsset(root, "webui/default/img/stop16.png");
	    	copyFileFromAsset(root, "webui/default/img/upload.png");

	    	temp = new File(root, "webui/dos/img");
	    	temp.mkdir();
	    	copyFileFromAsset(root, "webui/dos/img/cursor.gif");
	    	copyFileFromAsset(root, "webui/dos/img/fav.png");
	    	
    	}
    	catch(Exception ex){
    		root.delete();
    		ex.printStackTrace();
    	}
    }
    
    public void copy_config(){
    	if(!first_time) return;
    	try{
	    	File temp = new File(root, ".sharelin");
	    	if(!temp.exists())
	    		temp.mkdir();
	    	
	    	copyFileFromAsset(temp, "hubs.dat");
	    	copyFileFromAsset(temp, "share.dat");
	    	//read and write to a file
	    	copyFileFromAsset(temp, "sharelin.conf");
	    	//Complete = /home/tom/.sharelin/complete
	    	//Incomplete = /home/tom/.sharelin/incomplete
	    	FileWriter fw = new FileWriter(new File(temp, "sharelin.conf"), true);
	    	
	    	File complete = new File(temp, "complete");
	    	if(!complete.exists())
	    		complete.mkdir();
	    	File incomplete = new File(temp, "incomplete");
	    	if(!incomplete.exists())
	    		incomplete.mkdir();
	    	
	    	fw.write("Complete = ");
	    	fw.write(complete.getAbsolutePath());
	    	fw.write("\n");
	    	fw.write("Incomplete = ");
	    	fw.write(incomplete.getAbsolutePath());
	    	fw.write("\n");
	    	fw.close();
	    	
    	}
    	catch(Exception ex){
    		root.delete();
    		ex.printStackTrace();
    	};
    }
    
    private void copyFileFromAsset(File dest, String filename) throws IOException{
        File nf = new File(dest, filename);
        InputStream is = am.open(filename);
        if(!nf.createNewFile())
        	throw new IOException();
        OutputStream os = new FileOutputStream(nf);
        byte[] temp = new byte[256];
        int len;
        while((len = is.read(temp))> 0){
        	os.write(temp, 0, len);
        }
        is.close();
        os.close();
    }
    
    
    public boolean stop_server(){
    	try{
    		killCore();
    		server_thread.join();
    		server_thread = null;
    		removeNotification();
    	}
    	catch(Exception ex){
    		ex.printStackTrace();
    		return false;
    	}
    	return true;
    }
    
    static{
    	System.loadLibrary("sharelin");
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu){
    	MenuInflater inflater = getMenuInflater();
    	inflater.inflate(R.menu.main_menu, menu);
    	return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item){
    	switch(item.getItemId()){
    	case R.id.item1:
    		displayCredit();
    		return true;
    	default:
    		return super.onContextItemSelected(item);
    	}
    }
    
    public static void displayCredit(){
    	Toast.makeText(main._self, "This program is written by Tom Lai.\n" +
    			"Version 1.0\n" +
    			"\tCurrent only support searching and downloading, sharing is not enable in this version.\n" + 
    			"\nContact info:kongutoxiclab@gmail.com", Toast.LENGTH_LONG).show();
    }
    
    public void initNotification(){
		mNotificationManager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
		final Notification notifyDetails = new Notification(R.drawable.g2not,"DroidG2 is running",System.currentTimeMillis());
		  
		//long[] vibrate = {100,100,200,300};
		//notifyDetails.vibrate = vibrate;
		notifyDetails.flags |= Notification.FLAG_NO_CLEAR;
		Context context = getApplicationContext();
		
		 CharSequence contentTitle = "Droid2 is currently running.";
         CharSequence contentText = "Click to open.";
     
         Intent notifyIntent = new Intent(context, main.class);

         PendingIntent intent =
         PendingIntent.getActivity(main.this, 0,
         notifyIntent, android.content.Intent.FLAG_ACTIVITY_NEW_TASK);
         
         notifyDetails.setLatestEventInfo(context, contentTitle, contentText, intent);
	     
         mNotificationManager.notify(SIMPLE_NOTFICATION_ID, notifyDetails);
         
    }
    
    public void removeNotification(){
    	mNotificationManager.cancel(SIMPLE_NOTFICATION_ID);
    }
    
}
