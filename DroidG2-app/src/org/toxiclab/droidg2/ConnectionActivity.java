package org.toxiclab.droidg2;

import java.util.Date;
import java.util.TimerTask;

import android.app.Activity;
import android.os.Bundle;
import android.widget.ListView;
import android.widget.TextView;

public class ConnectionActivity extends Activity{
	private CA_Runnable r;
	private Thread t;
	private CA_Adapter adapter;
	private TextView tv;
	private ListView lv;
	public static String _connected;
	public static String _connecting;
	public static String _handshaking;
	
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.connection);
        adapter = new CA_Adapter(this, R.layout.connection);
        
        _connected = getString(R.string.cncned);
        _connecting = getString(R.string.cncning);
        _handshaking = getString(R.string.cnhdshake);
	
        r = new CA_Runnable();
        //setTitle("Connections");
        
        lv = (ListView)findViewById(R.id.listView1);
        tv = (TextView)findViewById(R.id.textView1);
        
        adapter = new CA_Adapter(this, R.layout.sever_item);
        lv.setAdapter(adapter);
    }
    
    public void onPause(){
    	super.onPause();
    	if(r != null)
    		//main.m_handler.removeCallbacks(r);
    		r.cancel();
    }
    
    public void onResume(){
    	super.onResume();
    	r = new CA_Runnable();
		main.m_timer.scheduleAtFixedRate(r, 1, 1000);
    }
    
    public void onStop(){
    	super.onStop();
    	if(r != null)
    		//main.m_handler.removeCallbacks(r);
    		r.cancel();
    }
    
    class CA_Runnable extends TimerTask{

    	@Override
    	public void run() {
    		Date x = new Date();
    		final Home h = MyHTMLParser.parseHome();
    		
    		main.m_handler.post(new Runnable(){
    			public void run(){
    				TextView tv = (TextView)findViewById(R.id.textView1);
		    		int cnt = 0;
		    		for(int i=0; i<h.status.size();++i){
		    			if(h.status.get(i).compareTo("connected") == 0)
		    				++cnt;
		    		}
    				tv.setText(String.format(getString(R.string.connect2), cnt));
    				adapter.updateFromHome(h);
    			}
    		});
    	}
    	
    }    
}

