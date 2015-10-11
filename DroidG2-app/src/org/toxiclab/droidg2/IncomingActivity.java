package org.toxiclab.droidg2;

import java.io.File;
import java.io.IOException;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;

public class IncomingActivity extends Activity {
    private IA_Runnable r = null;
    private IA_Adapter adapter = null;
    private ListView lv = null;
    private File contextfile = null;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.download);
        lv = (ListView)findViewById(R.id.ListView01);
        adapter = new IA_Adapter(this, R.layout.search_item);
        lv.setAdapter(adapter);
        registerForContextMenu(lv);
    }
    
    public void onPause(){
    	super.onPause();
    	if(r != null)
    		//main.m_handler.removeCallbacks(r);
    		r.cancel();
    }
    
    public void onResume(){
    	super.onResume();
    	r = new IA_Runnable();
		main.m_timer.scheduleAtFixedRate(r, 1, 1000);
    }
    
    public void onStop(){
    	super.onStop();
    	if(r != null)
    		//main.m_handler.removeCallbacks(r);
    		r.cancel();
    }
    
    class IA_Runnable extends TimerTask{

    	@Override
    	public void run() {
    		final Incoming h = MyHTMLParser.parseIncoming();
    		
    		//do the update here
    		main.m_handler.post(new Runnable(){
    			public void run(){
    				adapter.updateFromIncoming(h);
    			}
    		});
    	}
    	
    }    
    
	@Override
	public void onCreateContextMenu(ContextMenu menu, View v,
	    ContextMenuInfo menuInfo) {
	  if (v.getId()==R.id.ListView01) {
	    AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo)menuInfo;
	    
	    menu.setHeaderTitle("Actions");
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.incoming_context_menu, menu);
	    AdapterView.AdapterContextMenuInfo info1 = (AdapterView.AdapterContextMenuInfo)menuInfo;
	    IncomingFile se = adapter.getItem(info1.position);
	    contextfile  = new File(new File(se.parent), se.name);
	  }
	}
	
	@Override
	public boolean onContextItemSelected(MenuItem item) {
	  AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
	  switch (item.getItemId()) {
	  case R.id.item1:
		  openFile(contextfile);
		  return true;
	  case R.id.item2:
		  deleteFile(contextfile);
		  return true;
	  default:
	    return super.onContextItemSelected(item);
	  }
	}
	
	private boolean deleteFile(File f){
		if(f == null) return false;
		return f.delete();
	}
	
	private void openFile(File f){
		Intent intent = new Intent( Intent.ACTION_VIEW );
		File f1 = f;
		
		if(f1 == null) return;
		 
		String vlowerFileName = f.getName().toLowerCase();
		 
		if(  vlowerFileName.endsWith("mpg")
		  || vlowerFileName.endsWith("mp4")
		  )
		    intent.setDataAndType( Uri.fromFile(f1), "video/*" );
		else if( vlowerFileName.endsWith("mp3") )
		    intent.setDataAndType( Uri.fromFile(f1), "audio/*" );
		else if( vlowerFileName.endsWith("bmp")
		      || vlowerFileName.endsWith("gif")
		      || vlowerFileName.endsWith("jpg")
		      || vlowerFileName.endsWith("png")
		  )
		    intent.setDataAndType( Uri.fromFile(f1), "image/*" );
		else if( vlowerFileName.endsWith("txt")
		      || vlowerFileName.endsWith("html")
		      )
		    intent.setDataAndType( Uri.fromFile(f1), "text/*" );
		else if( vlowerFileName.endsWith("apk")
		      )
		    intent.setDataAndType( Uri.fromFile(f1), "application/vnd.android.package-archive" );
		else
		    intent.setDataAndType( Uri.fromFile(f1), "application/*" );
		
		startActivity(intent);
	}
}
